#include "haya/service/wifiman/event_handle.h"

const char *WIFIMAN_EVENT_TAG = "wifiman/event";

esp_err_t _hyWifimanEventRegister(HyWifiman *app)
{
    esp_err_t err = esp_event_handler_instance_register(
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        &_hyWifimanEventHandler,
        (void *)app,
        &app->wifi_evh);
    if (err != ESP_OK)
        return err;

    err = esp_event_handler_instance_register(
        IP_EVENT,
        ESP_EVENT_ANY_ID,
        &_hyWifimanEventHandler,
        (void *)app,
        &app->ip_evh);
    if (err != ESP_OK)
        return err;

    return ESP_OK;
}

esp_err_t _hyWifimanEventUnregister(HyWifiman *app)
{
    esp_err_t err = esp_event_handler_instance_unregister(
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        app->wifi_evh);
    if (err != ESP_OK)
        return err;

    err = esp_event_handler_instance_unregister(
        IP_EVENT,
        ESP_EVENT_ANY_ID,
        app->ip_evh);
    if (err != ESP_OK)
        return err;

    return ESP_OK;
}

void _hyWifimanEventHandler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data)
{
    HyWifiman *app = (HyWifiman *)arg;

    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
        case WIFI_EVENT_AP_START:
        {
            hyLogInfo(
                WIFIMAN_EVENT_TAG,
                "AP started");
            break;
        }

        case WIFI_EVENT_AP_STOP:
        {
            hyLogWarn(
                WIFIMAN_EVENT_TAG,
                "AP stopped");
            break;
        }

        case WIFI_EVENT_AP_STACONNECTED:
        {
            wifi_event_ap_staconnected_t *data = (wifi_event_ap_staconnected_t *)event_data;
            hyLogInfo(
                WIFIMAN_EVENT_TAG,
                "station device %02X:%02X:%02X:%02X:%02X:%02X joined, AID: %u",
                data->mac[0], data->mac[1], data->mac[2],
                data->mac[3], data->mac[4], data->mac[5],
                data->aid);
            break;
        }

        case WIFI_EVENT_AP_STADISCONNECTED:
        {
            wifi_event_ap_stadisconnected_t *data = (wifi_event_ap_stadisconnected_t *)event_data;
            hyLogWarn(
                WIFIMAN_EVENT_TAG,
                "station device %02X:%02X:%02X:%02X:%02X:%02X left, AID: %u",
                data->mac[0], data->mac[1], data->mac[2],
                data->mac[3], data->mac[4], data->mac[5],
                data->aid);
            break;
        }

        case WIFI_EVENT_STA_START:
        {
            hyLogInfo(
                WIFIMAN_EVENT_TAG,
                "STA started");
            break;
        }

        case WIFI_EVENT_STA_STOP:
        {
            hyLogWarn(
                WIFIMAN_EVENT_TAG,
                "STA stopped");
            break;
        }

        case WIFI_EVENT_STA_CONNECTED:
        {
            app->connecting = false;
            app->connected = true;
            app->reason = 0;

            wifi_event_sta_connected_t *data = (wifi_event_sta_connected_t *)event_data;
            hyLogInfo(
                WIFIMAN_EVENT_TAG,
                "STA connected to %.*s",
                data->ssid_len,
                (char *)data->ssid);
            break;
        }

        case WIFI_EVENT_STA_DISCONNECTED:
        {
            app->connecting = false;
            app->connected = false;

            wifi_event_sta_disconnected_t *data = (wifi_event_sta_disconnected_t *)event_data;
            app->reason = data->reason;
            hyLogWarn(
                WIFIMAN_EVENT_TAG,
                "STA disconnected from %.*s, reason: %u",
                data->ssid_len,
                (char *)data->ssid,
                data->reason);

            wifi_mode_t mode;
            esp_wifi_get_mode(&mode);
            if (mode == WIFI_MODE_STA)
                _hyWifimanFallback(app);
            break;
        }

        case WIFI_EVENT_SCAN_DONE:
        {
            app->scanning = false;
            wifi_event_sta_scan_done_t *data = (wifi_event_sta_scan_done_t *)event_data;
            hyLogInfo(
                WIFIMAN_EVENT_TAG,
                "WiFi scan done, got %u APs",
                data->number);
            break;
        }
        }
    }
    else if (event_base == IP_EVENT)
    {
        switch (event_id)
        {
        case IP_EVENT_STA_GOT_IP:
        {
            ip_event_got_ip_t *data = (ip_event_got_ip_t *)event_data;
            hyLogInfo(
                WIFIMAN_EVENT_TAG,
                "STA IP retrieved: " IPSTR,
                IP2STR(&data->ip_info.ip));
            break;
        }

        case IP_EVENT_STA_LOST_IP:
        {
            hyLogWarn(
                WIFIMAN_EVENT_TAG,
                "STA IP lost");
            break;
        }
        }
    }
}

void _hyWifimanFallback(HyWifiman *app)
{
    hyLogInfo(
        WIFIMAN_EVENT_TAG,
        "reverting to APSTA...");

    wifi_config_t ap_cfg = {0};
    wifi_config_t sta_cfg = {0};

    esp_err_t err = _hyWifimanNVSGetAPConfig(app, &ap_cfg);
    if (err != ESP_OK)
    {
        hyLogError(
            WIFIMAN_EVENT_TAG,
            "failed to get AP config: %s",
            esp_err_to_name(err));
        return;
    }

    err = _hyWifimanNVSGetSTAConfig(app, &sta_cfg);
    if (err != ESP_OK)
    {
        hyLogError(
            WIFIMAN_EVENT_TAG,
            "failed to get STA config: %s",
            esp_err_to_name(err));
        return;
    }

    err = esp_wifi_set_mode(WIFI_MODE_APSTA);
    if (err != ESP_OK)
    {
        hyLogError(
            WIFIMAN_EVENT_TAG,
            "failed to set APSTA mode: %s",
            esp_err_to_name(err));
        return;
    }

    err = esp_wifi_set_config(WIFI_IF_AP, &ap_cfg);
    if (err != ESP_OK)
    {
        hyLogError(
            WIFIMAN_EVENT_TAG,
            "failed to set AP config: %s",
            esp_err_to_name(err));
        return;
    }

    err = esp_wifi_set_config(WIFI_IF_STA, &sta_cfg);
    if (err != ESP_OK)
    {
        hyLogError(
            WIFIMAN_EVENT_TAG,
            "failed to set STA config: %s",
            esp_err_to_name(err));
        return;
    }

    err = esp_wifi_connect();
    if (err != ESP_OK)
    {
        hyLogError(
            WIFIMAN_EVENT_TAG,
            "failed to connect: %s",
            esp_err_to_name(err));
        return;
    }

    hyLogInfo(
        WIFIMAN_EVENT_TAG,
        "APSTA restart success");
}