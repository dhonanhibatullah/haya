#include "core/service.h"

const char *CORE_SERVICE_TAG = "core/service";

void coreServiceConfig(CoreService *s, CorePeripheral *p)
{
    esp_err_t err;
    HyErr herr;

#if SCG_HTTP_SERVER_ENABLE == 1
#ifndef SERVICE_USED
#define SERVICE_USED
#endif

    err = hyHttpServerSetup(&s->server, SCG_HTTP_SERVER_PORT);
    if (err != ESP_OK)
    {
        hyLogError(
            CORE_SERVICE_TAG,
            "failed to setup HTTP server: %s",
            esp_err_to_name(err));
        goto restart_device;
    }

    hyLogInfo(
        CORE_SERVICE_TAG,
        "HTTP server listening at port %d",
        SCG_HTTP_SERVER_PORT);
#endif

#if SCG_WIFIMAN_ENABLE == 1
#ifndef SERVICE_USED
#define SERVICE_USED
#endif

    s->wifiman = hyWifimanNew(
        p->nvs,
        s->server,
        p->wifi_ap_netif,
        p->wifi_sta_netif,
        SCG_WIFIMAN_DEFAULT_AP_SSID,
        SCG_WIFIMAN_DEFAULT_AP_PASS);
    if (s->wifiman == NULL)
    {
        hyLogError(
            CORE_SERVICE_TAG,
            "failed to create Wifiman service");
        goto restart_device;
    }

    herr = hyWifimanStart(s->wifiman);
    if (herr != HY_ERR_NONE)
    {
        hyLogError(
            CORE_SERVICE_TAG,
            "failed to start Wifiman service: %s",
            hyErrToStr(herr));
        goto restart_device;
    }

    hyLogInfo(
        CORE_SERVICE_TAG,
        "Wifiman service started");
#endif

#if SCG_MQTT_CLIENT_ENABLE == 1
#ifndef SERVICE_USED
#define SERVICE_USED
#endif

#endif

    return;

#ifdef SERVICE_USED
restart_device:
    hyLogWarn(
        CORE_SERVICE_TAG,
        "restarting device in 3s...");
    vTaskDelay(pdMS_TO_TICKS(3000));
    esp_restart();
#endif
}