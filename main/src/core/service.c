#include "core/service.h"

const char *CORE_SERVICE_TAG = "core/service";

void coreServiceConfig(CoreService *s, CorePeripheral *p)
{
    HyErr herr;

#if PCG_WIFIMAN_ENABLE == 1
#ifndef SERVICE_USED
#define SERVICE_USED
#endif

    s->wifiman = wifimanNew(
        p->nvs,
        p->wifi_ap_netif,
        p->wifi_sta_netif,
        PCG_WIFIMAN_DEFAULT_AP_SSID,
        PCG_WIFIMAN_DEFAULT_AP_PASS);
    if (s->wifiman == NULL)
    {
        hyLogError(
            CORE_SERVICE_TAG,
            "failed to create Wifiman");
        goto restart_device;
    }
    herr = hyAppStart(s->wifiman->app_hdl);
    if (herr != HY_ERR_NONE)
    {
        hyLogError(
            CORE_SERVICE_TAG,
            "failed to start Wifiman service: %s",
            hyErrToStr(herr));
        goto restart_device;
    }
#endif

#ifdef SERVICE_USED
restart_device:
    hyLogWarn(
        CORE_SERVICE_TAG,
        "restarting device in 3s...");
    vTaskDelay(pdMS_TO_TICKS(3000));
    esp_restart();
#endif
}