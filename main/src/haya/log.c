#include "haya/log.h"

#define _HY_LOG_TAKE_MUTEX() xSemaphoreTake(_hy_log.mtx, portMAX_DELAY)
#define _HY_LOG_GIVE_MUTEX() xSemaphoreGive(_hy_log.mtx)

_HyLogHandle _hy_log = {
    .mtx = NULL,
    .def_vprintf = NULL,
    .setup = false,

    .save_en = false,
    .dir_path = NULL,
    .bin_path = NULL,
    .log_path_fmt = NULL,
    .file_keep_num = 0,
    .rotation_period = 0,
    .recover_period = 0,
    .f = NULL,
    .is_err = false,
    .check_ts = 0,

    .cb_arg = NULL,
    .cb_func = NULL};

HyErr hyLogSetup()
{
    if (_hy_log.mtx == NULL)
    {
        _hy_log.mtx = xSemaphoreCreateMutex();
        if (_hy_log.mtx == NULL)
            return HY_ERR_FAILURE;
    }
    _hy_log.def_vprintf = esp_log_set_vprintf(_hyLogWrapper);
    _hy_log.setup = true;
    return HY_ERR_NONE;
}

HyErr hyLogSaveEnable(
    const char *dir_path,
    uint32_t file_keep_num,
    TickType_t rotation_period,
    TickType_t recover_period)
{
    // ignore if setup is not called
    if (!_hy_log.setup)
        return HY_ERR_FAILURE;

    _HY_LOG_TAKE_MUTEX();

    // ignore if the save is already enabled
    if (_hy_log.save_en)
    {
        _HY_LOG_GIVE_MUTEX();
        return HY_ERR_NONE;
    }

    // ignore if the args are bad
    if (dir_path == NULL ||
        rotation_period < pdMS_TO_TICKS(5 * 1000 * 60) ||
        file_keep_num == 0 ||
        recover_period == 0 ||
        dir_path[strlen(dir_path) - 1] == '/')
    {
        _HY_LOG_GIVE_MUTEX();
        return HY_ERR_BAD_ARGS;
    }

    // save the passed arguments
    _hy_log.rotation_period = rotation_period;
    _hy_log.file_keep_num = file_keep_num;
    _hy_log.recover_period = recover_period;

    // save the dir path
    size_t dir_path_len = strlen(dir_path);

    _hy_log.dir_path = (char *)malloc(dir_path_len + 1);
    if (_hy_log.dir_path == NULL)
    {
        _HY_LOG_GIVE_MUTEX();
        return HY_ERR_MALLOC_FAILED;
    }

    strcpy(_hy_log.dir_path, dir_path);

    // construct and save the log bin path
    const char *bin_filename = "/.log.bin";
    size_t bin_path_len = dir_path_len + strlen(bin_filename);

    _hy_log.bin_path = (char *)malloc(bin_path_len + 1);
    if (_hy_log.bin_path == NULL)
    {
        _hyLogFreePath();
        _HY_LOG_GIVE_MUTEX();
        return HY_ERR_MALLOC_FAILED;
    }

    snprintf(_hy_log.bin_path, bin_path_len + 1, "%s%s", dir_path, bin_filename);

    // construct and save the log filename format path
    const char *log_filename_fmt = "/%010d.log";
    size_t log_path_fmt_len = dir_path_len + strlen(log_filename_fmt);

    _hy_log.log_path_fmt = (char *)malloc(log_path_fmt_len + 1);
    if (_hy_log.log_path_fmt == NULL)
    {
        _hyLogFreePath();
        _HY_LOG_GIVE_MUTEX();
        return HY_ERR_MALLOC_FAILED;
    }

    snprintf(_hy_log.log_path_fmt, log_path_fmt_len + 1, "%s%s", dir_path, log_filename_fmt);

    // create the directory if not exist
    int res = mkdir(_hy_log.dir_path, 0755);
    if (res != 0 && errno != EEXIST)
    {
        _hyLogFreePath();
        _HY_LOG_GIVE_MUTEX();
        return HY_ERR_FAILURE;
    }

    // check for bin file existence
    struct stat st;
    if (stat(_hy_log.bin_path, &st) != 0)
    {
        FILE *f = fopen(_hy_log.bin_path, "wb");
        if (!f)
        {
            _hyLogFreePath();
            _HY_LOG_GIVE_MUTEX();
            return HY_ERR_FAILURE;
        }

        int cnt[2] = {-1, 0};
        if (fwrite(&cnt, sizeof(int), 2, f) != 2)
        {
            fclose(f);
            _hyLogFreePath();
            _HY_LOG_GIVE_MUTEX();
            return HY_ERR_FAILURE;
        }

        fflush(f);
        fclose(f);
    }

    // set the save enable flag into true
    _hy_log.save_en = true;
    _HY_LOG_GIVE_MUTEX();
    return HY_ERR_NONE;
}

void hyLogSaveDisable()
{
    if (!_hy_log.setup)
        return;

    _HY_LOG_TAKE_MUTEX();

    if (!_hy_log.save_en)
    {
        _HY_LOG_GIVE_MUTEX();
        return;
    }

    _hyLogFreePath();
    _hy_log.save_en = false;

    _HY_LOG_GIVE_MUTEX();
}

void hyLogSetCallback(HyLogCbFunc cb, void *arg)
{
    _HY_LOG_TAKE_MUTEX();

    _hy_log.cb_func = cb;
    _hy_log.cb_arg = arg;

    _HY_LOG_GIVE_MUTEX();
}

int _hyLogWrapper(const char *format, va_list args)
{
    if (!_hy_log.setup)
        return -1;

    _HY_LOG_TAKE_MUTEX();

    if (_hy_log.save_en)
    {
        va_list args_cp;
        va_copy(args_cp, args);
        _hyLogSaveHandle(format, args_cp);
        va_end(args_cp);
    }

    if (_hy_log.cb_func != NULL)
    {
        va_list args_cp;
        va_copy(args_cp, args);
        _hy_log.cb_func(_hy_log.cb_arg, format, args_cp);
        va_end(args_cp);
    }

    int ret = _hy_log.def_vprintf(format, args);

    _HY_LOG_GIVE_MUTEX();
    return ret;
}

void _hyLogFreePath()
{
    free(_hy_log.dir_path);
    _hy_log.dir_path = NULL;
    free(_hy_log.bin_path);
    _hy_log.bin_path = NULL;
    free(_hy_log.log_path_fmt);
    _hy_log.log_path_fmt = NULL;
}

void _hyLogSaveHandle(const char *format, va_list args)
{
    TickType_t now_ts = xTaskGetTickCount();

    if (_hy_log.is_err)
    {
        if (now_ts - _hy_log.check_ts < _hy_log.recover_period)
            return;
        _hy_log.check_ts = now_ts;
    }

    if ((_hy_log.is_err) ||
        (now_ts - _hy_log.check_ts >= _hy_log.rotation_period))
    {
        if (_hy_log.f != NULL)
        {
            fclose(_hy_log.f);
            _hy_log.f = NULL;
        }

        int file_idx = _hyLogNextIndex();
        if (file_idx == -1)
            return;

        char filename[_HY_LOG_PATH_MAX_LEN];
        snprintf(filename, _HY_LOG_PATH_MAX_LEN, _hy_log.log_path_fmt, file_idx);
        _hy_log.f = fopen(filename, "wb");
        if (_hy_log.f)
        {
            _hy_log.check_ts = now_ts;
            _hy_log.is_err = false;
        }
    }

    if (_hy_log.f != NULL)
    {
        int res = vfprintf(_hy_log.f, format, args);
        if (res <= 0)
        {
            fclose(_hy_log.f);
            _hy_log.f = NULL;
            _hy_log.is_err = true;
            _hy_log.check_ts = now_ts;
        }
    }
}

int _hyLogNextIndex()
{
    FILE *f = fopen(_hy_log.bin_path, "rb+");
    if (!f)
        return -1;

    int cnt[2];
    if (fread(&cnt, sizeof(int), 2, f) != 2)
    {
        fclose(f);
        return -1;
    }
    cnt[1] += 1;

    if (cnt[1] - cnt[0] > _hy_log.file_keep_num)
    {
        char filename[_HY_LOG_PATH_MAX_LEN];
        snprintf(filename, _HY_LOG_PATH_MAX_LEN, _hy_log.log_path_fmt, cnt[0]);
        remove(filename);
        cnt[0] += 1;
    }

    if (fseek(f, 0, SEEK_SET) != 0)
    {
        fclose(f);
        return -1;
    }
    if (fwrite(&cnt, sizeof(int), 2, f) != 2)
    {
        fclose(f);
        return -1;
    }

    fflush(f);
    fclose(f);
    return cnt[1];
}