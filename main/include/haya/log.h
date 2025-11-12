#ifndef __HAYA_LOG_H
#define __HAYA_LOG_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "haya/error.h"

#define hyLogError ESP_LOGE
#define hyLogWarn ESP_LOGW
#define hyLogInfo ESP_LOGI
#define hyLogDebug ESP_LOGD
#define hyLogVerbose ESP_LOGV

#define _HY_LOG_PATH_MAX_LEN 256

typedef void (*HyLogCbFunc)(void *arg, const char *, va_list);

typedef struct
{
    SemaphoreHandle_t mtx;
    vprintf_like_t def_vprintf;
    volatile bool setup;

    bool save_en;
    char *dir_path;
    char *bin_path;
    char *log_path_fmt;
    uint32_t file_keep_num;
    TickType_t rotation_period;
    TickType_t recover_period;
    FILE *f;
    bool is_err;
    uint32_t check_ts;

    void *cb_arg;
    HyLogCbFunc cb_func;
} _HyLogHandle;

extern _HyLogHandle _hy_log;

HyErr hyLogSetup();

HyErr hyLogSaveEnable(
    const char *dir_path,
    TickType_t file_rotation_period,
    uint32_t file_keep_num,
    TickType_t recover_period);

void hyLogSaveDisable();

void hyLogSetCallback(HyLogCbFunc cb, void *arg);

int _hyLogWrapper(const char *format, va_list args);

void _hyLogFreePath();

void _hyLogSaveHandle(const char *format, va_list args);

int _hyLogNextIndex();

#endif