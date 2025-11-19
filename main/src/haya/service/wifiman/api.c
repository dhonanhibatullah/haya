#include "haya/service/wifiman/api.h"

const char *WIFIMAN_TAG = "wifiman";

HyWifiman *hyWifimanNew(
    nvs_handle_t nvs,
    httpd_handle_t server,
    esp_netif_t *ap_netif,
    esp_netif_t *sta_netif,
    const char *ap_default_ssid,
    const char *ap_default_pass)
{
    HyWifiman *new = (HyWifiman *)malloc(sizeof(HyWifiman));
    if (new == NULL)
        return NULL;

    new->nvs = nvs;
    new->server = server;

    new->ap_netif = ap_netif;
    new->sta_netif = sta_netif;
    new->ap_default_ssid = ap_default_ssid;
    new->ap_default_pass = ap_default_pass;

    new->wifi_evh = NULL;
    new->ip_evh = NULL;

    new->scanning = false;
    new->connecting = false;
    new->connected = false;

    return new;
}

HyErr hyWifimanStart(HyWifiman *wm)
{
    esp_err_t err = _hyWifimanNVSInit(wm);
    if (err != ESP_OK)
    {
        hyLogError(
            WIFIMAN_TAG,
            "failed to initiate NVS: %s",
            esp_err_to_name(err));
        return HY_ERR_FAILURE;
    }

    err = _hyWifimanEventRegister(wm);
    if (err != ESP_OK)
    {
        hyLogError(
            WIFIMAN_TAG,
            "failed to register events: %s",
            esp_err_to_name(err));
        return HY_ERR_FAILURE;
    }

    err = _hyWifimanWiFiInit(wm);
    if (err != ESP_OK)
    {
        hyLogError(
            WIFIMAN_TAG,
            "failed to initiate WiFi: %s",
            esp_err_to_name(err));

        _hyWifimanEventUnregister(wm);
        return HY_ERR_FAILURE;
    }

    err = _hyWifimanHTTPHandleRoute(wm->server, (void *)wm);
    if (err != ESP_OK)
    {
        hyLogError(
            WIFIMAN_TAG,
            "failed to route HTTP: %s",
            esp_err_to_name(err));

        _hyWifimanWiFiDeinit();
        _hyWifimanEventUnregister(wm);
        return HY_ERR_FAILURE;
    }

    hyLogInfo(
        WIFIMAN_TAG,
        "started successfully at /wifiman");
    return HY_ERR_NONE;
}

HyErr hyWifimanStop(HyWifiman *wm)
{
    return HY_ERR_NONE;
}