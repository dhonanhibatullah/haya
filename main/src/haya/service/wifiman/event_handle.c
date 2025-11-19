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
                "WiFi AP started");
            break;
        }

        case WIFI_EVENT_AP_STOP:
        {
            hyLogWarn(
                WIFIMAN_EVENT_TAG,
                "WiFi AP stopped");
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
                "WiFi STA started");
            break;
        }

        case WIFI_EVENT_STA_STOP:
        {
            hyLogWarn(
                WIFIMAN_EVENT_TAG,
                "WiFi STA stopped");
            break;
        }

        case WIFI_EVENT_STA_CONNECTED:
        {
            app->connecting = false;
            app->connected = true;
            wifi_event_sta_connected_t *data = (wifi_event_sta_connected_t *)event_data;
            hyLogInfo(
                WIFIMAN_EVENT_TAG,
                "WiFi STA connected to %.*s",
                data->ssid_len,
                (char *)data->ssid);
            break;
        }

        case WIFI_EVENT_STA_DISCONNECTED:
        {
            app->connecting = false;
            app->connected = false;
            wifi_event_sta_disconnected_t *data = (wifi_event_sta_disconnected_t *)event_data;
            hyLogWarn(
                WIFIMAN_EVENT_TAG,
                "WiFi STA disconnected from %.*s, reason: %u",
                data->ssid_len,
                (char *)data->ssid,
                data->reason);
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