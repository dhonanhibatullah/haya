#include "haya/app.h"

QueueHandle_t _app_exit_q = NULL;

HyAppHandle *hyAppNew(HyAppConfig *cfg)
{
    if (_app_exit_q == NULL || cfg == NULL)
        return NULL;

    HyAppHandle *new = (HyAppHandle *)malloc(sizeof(HyAppHandle));
    if (new == NULL)
        return NULL;

    new->_ev = xEventGroupCreate();
    if (new->_ev == NULL)
    {
        free(new);
        return NULL;
    }

    new->_th = NULL;

    new->_cfg.name = (char *)malloc(strlen(cfg->name) + 1);
    if (new->_cfg.name == NULL)
    {
        vEventGroupDelete(new->_ev);
        free(new);
        return NULL;
    }
    strcpy(new->_cfg.name, cfg->name);

    new->_cfg.priority = cfg->priority;
    new->_cfg.stack_size = cfg->stack_size;
    new->_cfg.sleep_tick = cfg->sleep_tick;
    new->_cfg.param = cfg->param;

    new->_cb.on_setup = NULL;
    new->_cb.on_loop = NULL;
    new->_cb.on_paused = NULL;
    new->_cb.on_resumed = NULL;
    new->_cb.on_stopped = NULL;

    new->_exit_code = HY_APP_EXIT_CODE_NONE;
    new->_ok = false;
    new->_sus = false;

    return new;
}

HyErr hyAppSetCallbackGroup(HyAppHandle *h, HyAppCallbackGroup *cb)
{
    if (h == NULL || cb == NULL)
        return HY_ERR_BAD_ARGS;

    h->_cb.on_setup = cb->on_setup;
    h->_cb.on_loop = cb->on_loop;
    h->_cb.on_paused = cb->on_paused;
    h->_cb.on_resumed = cb->on_resumed;
    h->_cb.on_stopped = cb->on_stopped;

    return HY_ERR_NONE;
}

HyErr hyAppStart(HyAppHandle *h)
{
    if (h == NULL)
        return HY_ERR_BAD_ARGS;

    h->_ok = true;

    if (xTaskCreate(
            _hyAppTaskWrapper,
            h->_cfg.name,
            h->_cfg.stack_size,
            h,
            h->_cfg.priority,
            &h->_th) != pdPASS)
    {
        h->_ok = false;
        return HY_ERR_FAILURE;
    }

    EventBits_t ev_bits = xEventGroupWaitBits(
        h->_ev,
        _HY_APP_EVENT_RUNNING_BIT,
        pdFALSE,
        pdFALSE,
        _HY_APP_START_TIMEOUT);
    if ((ev_bits & _HY_APP_EVENT_RUNNING_BIT) != _HY_APP_EVENT_RUNNING_BIT)
    {
        vTaskDelete(h->_th);
        h->_th = NULL;
        h->_ok = false;
        return HY_ERR_TIMEOUT;
    }

    return HY_ERR_NONE;
}

HyErr hyAppStop(HyAppHandle *h)
{
    if (h == NULL)
        return HY_ERR_BAD_ARGS;

    h->_ok = false;

    EventBits_t ev_bits = xEventGroupWaitBits(
        h->_ev,
        _HY_APP_EVENT_STOPPED_BIT,
        pdTRUE,
        pdFALSE,
        (h->_cfg.sleep_tick * 2) + _HY_APP_TOLERANCE_DELAY);
    if ((ev_bits & _HY_APP_EVENT_STOPPED_BIT) != _HY_APP_EVENT_STOPPED_BIT)
    {
        vTaskDelete(h->_th);
        h->_th = NULL;
        return HY_ERR_TIMEOUT;
    }

    h->_th = NULL;
    return HY_ERR_NONE;
}

HyErr hyAppPause(HyAppHandle *h)
{
    if (h == NULL)
        return HY_ERR_BAD_ARGS;

    if ((xEventGroupGetBits(h->_ev) & _HY_APP_EVENT_PAUSED_BIT) == _HY_APP_EVENT_PAUSED_BIT)
        return HY_ERR_NONE;

    h->_sus = true;

    EventBits_t ev_bits = xEventGroupWaitBits(
        h->_ev,
        _HY_APP_EVENT_PAUSED_BIT,
        pdFALSE,
        pdFALSE,
        (h->_cfg.sleep_tick * 2) + _HY_APP_TOLERANCE_DELAY);
    if ((ev_bits & _HY_APP_EVENT_PAUSED_BIT) != _HY_APP_EVENT_PAUSED_BIT)
        return HY_ERR_TIMEOUT;

    return HY_ERR_NONE;
}

HyErr hyAppResume(HyAppHandle *h)
{
    if (h == NULL)
        return HY_ERR_BAD_ARGS;

    if ((xEventGroupGetBits(h->_ev) & _HY_APP_EVENT_PAUSED_BIT) != _HY_APP_EVENT_PAUSED_BIT)
        return HY_ERR_NONE;

    xEventGroupSetBits(h->_ev, _HY_APP_EVENT_RESUME_CMD_BIT);

    EventBits_t ev_bits = xEventGroupWaitBits(
        h->_ev,
        _HY_APP_EVENT_RESUMED_BIT,
        pdTRUE,
        pdFALSE,
        (h->_cfg.sleep_tick * 2) + _HY_APP_TOLERANCE_DELAY);
    if ((ev_bits & _HY_APP_EVENT_RESUMED_BIT) != _HY_APP_EVENT_RESUMED_BIT)
        return HY_ERR_TIMEOUT;

    return HY_ERR_NONE;
}

HyAppExitCode hyAppExitCode(HyAppHandle *h)
{
    if (h == NULL)
        return HY_APP_EXIT_CODE_NONE;

    return h->_exit_code;
}

void _hyAppTaskWrapper(void *pvParameter)
{
    HyAppHandle *h = (HyAppHandle *)pvParameter;
    bool sus_notif = false;

    xEventGroupSetBits(h->_ev, _HY_APP_EVENT_RUNNING_BIT);

    if (h->_cb.on_setup != NULL)
    {
        h->_exit_code = h->_cb.on_setup(h->_cfg.param);
        if (h->_exit_code != HY_APP_EXIT_CODE_NONE)
            h->_ok = false;
    }

    if (h->_ok)
        vTaskDelay(_HY_APP_AFTER_SETUP_DELAY);

    while (h->_ok)
    {
        if (h->_sus)
        {
            if (!sus_notif)
            {
                sus_notif = true;
                xEventGroupSetBits(h->_ev, _HY_APP_EVENT_PAUSED_BIT);

                if (h->_cb.on_paused != NULL)
                    h->_cb.on_paused(h->_cfg.param);
            }

            EventBits_t bits = xEventGroupWaitBits(
                h->_ev,
                _HY_APP_EVENT_RESUME_CMD_BIT,
                pdTRUE,
                pdFALSE,
                h->_cfg.sleep_tick);
            if ((bits & _HY_APP_EVENT_RESUME_CMD_BIT) == _HY_APP_EVENT_RESUME_CMD_BIT)
            {
                h->_sus = false;
                sus_notif = false;
                xEventGroupClearBits(h->_ev, _HY_APP_EVENT_PAUSED_BIT);
                xEventGroupSetBits(h->_ev, _HY_APP_EVENT_RESUMED_BIT);

                if (h->_cb.on_resumed != NULL)
                    h->_cb.on_resumed(h->_cfg.param);
            }

            continue;
        }

        if (h->_cb.on_loop != NULL)
        {
            h->_exit_code = h->_cb.on_loop(h->_cfg.param);
            if (h->_exit_code != HY_APP_EXIT_CODE_NONE)
            {
                h->_ok = false;
                break;
            }
        }

        vTaskDelay(h->_cfg.sleep_tick);
    }

    xEventGroupClearBits(h->_ev, _HY_APP_EVENT_RUNNING_BIT);
    xEventGroupSetBits(h->_ev, _HY_APP_EVENT_STOPPED_BIT);

    if (h->_cb.on_stopped != NULL)
        h->_cb.on_stopped(h->_cfg.param);

    xQueueSend(_app_exit_q, &h, 0);
    vTaskDelete(NULL);
}

bool _hyAppInit()
{
    _app_exit_q = xQueueCreate(_HY_APP_EXIT_QUEUE_LEN, sizeof(HyAppHandle *));
    if (_app_exit_q == NULL)
        return false;
    return true;
}

HyErr _hyAppDelete(HyAppHandle *h)
{
    if (h == NULL)
        return HY_ERR_BAD_ARGS;

    free(h->_cfg.name);
    h->_cfg.name = NULL;

    if (h->_ev != NULL)
    {
        vEventGroupDelete(h->_ev);
        h->_ev = NULL;
    }

    free(h);
    return HY_ERR_NONE;
}