#include "haya/peripheral/wifi.h"

esp_err_t hyPeripheralWiFiSetup(
    nvs_handle_t nvs,
    esp_netif_t **ap_netif,
    const char *ap_ssid,
    const char *ap_pass,
    int channel,
    int max_connection)
{
    if (ap_netif == NULL || ap_ssid == NULL || ap_pass == NULL)
        return ESP_ERR_INVALID_ARG;

    size_t ssid_len = strlen(ap_ssid);
    size_t pass_len = strlen(ap_pass);
    if (ssid_len == 0 || ssid_len > 32 || pass_len > 64)
        return ESP_ERR_INVALID_ARG;

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_err_t err = esp_wifi_init(&cfg);
    if (err != ESP_OK)
        return err;

    *ap_netif = esp_netif_create_default_wifi_ap();
    if (*ap_netif == NULL)
    {
        esp_wifi_deinit();
        return ESP_FAIL;
    }

    wifi_config_t wifi_cfg = {
        .ap = {
            .channel = channel,
            .max_connection = max_connection,
            .authmode = WIFI_AUTH_OPEN,
        }};

    size_t saved_ssid_len = 32;
    err = nvs_get_str(nvs, "wifi-ap-ssid", (char *)wifi_cfg.ap.ssid, &wifi_cfg.ap.ssid_len);
    if (err != ESP_OK)
    {
        strncpy((char *)wifi_cfg.ap.ssid, ap_ssid, ssid_len);
        wifi_cfg.ap.ssid_len = ssid_len;
    }

    size_t saved_pass_len = 64;
    err = nvs_get_str(nvs, "wifi-ap-pass", (char *)wifi_cfg.ap.ssid, &saved_pass_len);
    if (err == ESP_OK)
    {
        if (saved_pass_len > 0)
            wifi_cfg.ap.authmode = WIFI_AUTH_WPA2_PSK;
    }
    else if (pass_len > 0)
    {
        wifi_cfg.ap.authmode = WIFI_AUTH_WPA2_PSK;
        strncpy((char *)wifi_cfg.ap.password, ap_pass, pass_len);
    }

    err = esp_wifi_set_mode(WIFI_MODE_AP);
    if (err != ESP_OK)
    {
        esp_netif_destroy(*ap_netif);
        esp_wifi_deinit();
        *ap_netif = NULL;
        return err;
    }

    err = esp_wifi_set_config(WIFI_IF_AP, &wifi_cfg);
    if (err != ESP_OK)
    {
        esp_wifi_set_mode(WIFI_MODE_NULL);
        esp_netif_destroy(*ap_netif);
        esp_wifi_deinit();
        *ap_netif = NULL;
        return err;
    }

    err = esp_wifi_start();
    if (err != ESP_OK)
    {
        esp_wifi_set_mode(WIFI_MODE_NULL);
        esp_netif_destroy(*ap_netif);
        esp_wifi_deinit();
        *ap_netif = NULL;
        return err;
    }

    return ESP_OK;
}