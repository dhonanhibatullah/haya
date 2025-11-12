#include "core/handle.h"

const char *CORE_HANDLE_TAG = "core/handle";

void coreSetup(Core *core)
{
    HyErr err = hyLogSetup();
    if (err != HY_ERR_NONE)
    {
        hyLogError(
            CORE_HANDLE_TAG,
            "failed to setup log: %s",
            hyErrToStr(err));
        coreRestart(3000);
    }
    hyLogInfo(
        CORE_HANDLE_TAG,
        "log setup success");

    coreInterfaceConfig(core->i);
}

void coreLoop(Core *core)
{
    HyAppHandle *app_handle;
    while (1)
    {
        xQueueReceive(_hy_app_exit_q, &app_handle, portMAX_DELAY);
        switch (app_handle->_cfg.exit_action)
        {
        case HY_APP_EXIT_ACTION_FREE:
            _coreFreeApp(app_handle);
            break;

        case HY_APP_EXIT_ACTION_RESTART:
            _coreRestartApp(app_handle);
            break;

        case HY_APP_EXIT_ACTION_RESTART_UNLESS_STOPPED:
            if (app_handle->_exit_code == HY_APP_EXIT_CODE_NONE)
                _coreFreeApp(app_handle);
            else
                _coreRestartApp(app_handle);
            break;

        case HY_APP_EXIT_ACTION_RESTART_DEVICE:
            hyLogWarn(
                CORE_HANDLE_TAG,
                "%s app requested a device restart",
                app_handle->_cfg.name);
            _coreFreeApp(app_handle);
            coreRestart(3000);
            break;
        }
    }
}

void coreRestart(uint32_t wait_ms)
{
    hyLogInfo(
        CORE_HANDLE_TAG,
        "restarting device in %u ms...",
        wait_ms);
    vTaskDelay(pdMS_TO_TICKS(wait_ms));
    esp_restart();
}

void _coreFreeApp(HyAppHandle *app_handle)
{
    hyLogInfo(
        CORE_HANDLE_TAG,
        "%s app freed",
        app_handle->_cfg.name);
    _hyAppDelete(app_handle);
}

void _coreRestartApp(HyAppHandle *app_handle)
{
    hyLogInfo(
        CORE_HANDLE_TAG,
        "restarting %s app...",
        app_handle->_cfg.name);

    app_handle->_exit_code = HY_APP_EXIT_CODE_NONE;
    xEventGroupClearBits(app_handle->_ev, _HY_APP_EVENT_STOPPED_BIT);

    HyErr err = hyAppStart(app_handle);
    if (err != HY_ERR_NONE)
    {
        hyLogError(
            CORE_HANDLE_TAG,
            "failed to restart %s app: %s, restarting device instead...",
            app_handle->_cfg.name,
            hyErrToStr(err));

        _coreFreeApp(app_handle);
        coreRestart(3000);
    }
    hyLogInfo(
        CORE_HANDLE_TAG,
        "%s app restarted successfully",
        app_handle->_cfg.name);
}