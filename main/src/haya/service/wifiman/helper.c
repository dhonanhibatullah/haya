#include "haya/service/wifiman/helper.h"

const char *WIFIMAN_AP_SSID_KEY = "wm_ap_ssid";
const char *WIFIMAN_AP_PASS_KEY = "wm_ap_pass";
const char *WIFIMAN_STA_SSID_KEY = "wm_sta_ssid";
const char *WIFIMAN_STA_PASS_KEY = "wm_sta_pass";

esp_err_t _hyWifimanNVSInit(HyWifiman *app)
{
    if (app == NULL)
        return ESP_ERR_INVALID_ARG;

    size_t def_ssid_len = strlen(app->ap_default_ssid);
    size_t def_pass_len = strlen(app->ap_default_pass);

    if ((def_ssid_len == 0 || def_ssid_len > 32) &&
        (def_pass_len < 8 || def_pass_len > 64))
        return ESP_ERR_INVALID_SIZE;

    esp_err_t err = nvs_find_key(app->nvs, WIFIMAN_AP_SSID_KEY, NULL);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        err = nvs_set_str(app->nvs, WIFIMAN_AP_SSID_KEY, app->ap_default_ssid);
        if (err != ESP_OK)
            return err;
    }
    else if (err != ESP_OK)
        return err;

    err = nvs_find_key(app->nvs, WIFIMAN_AP_PASS_KEY, NULL);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        err = nvs_set_str(app->nvs, WIFIMAN_AP_PASS_KEY, app->ap_default_pass);
        if (err != ESP_OK)
            return err;
    }
    else if (err != ESP_OK)
        return err;

    err = nvs_find_key(app->nvs, WIFIMAN_STA_SSID_KEY, NULL);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        err = nvs_set_str(app->nvs, WIFIMAN_STA_SSID_KEY, "");
        if (err != ESP_OK)
            return err;
    }
    else if (err != ESP_OK)
        return err;

    err = nvs_find_key(app->nvs, WIFIMAN_STA_PASS_KEY, NULL);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        err = nvs_set_str(app->nvs, WIFIMAN_STA_PASS_KEY, "");
        if (err != ESP_OK)
            return err;
    }
    else if (err != ESP_OK)
        return err;

    err = nvs_commit(app->nvs);
    if (err != ESP_OK)
        return err;

    return ESP_OK;
}

esp_err_t _hyWifimanNVSGetAPConfig(HyWifiman *app, wifi_config_t *wifi_cfg)
{
    if (app == NULL || wifi_cfg == NULL)
        return ESP_ERR_INVALID_ARG;

    size_t ssid_len;
    size_t pass_len;

    wifi_cfg->ap.channel = 1;
    wifi_cfg->ap.max_connection = 4;
    wifi_cfg->ap.authmode = WIFI_AUTH_OPEN;

    ssid_len = 32;
    esp_err_t err = nvs_get_str(app->nvs, WIFIMAN_AP_SSID_KEY, (char *)wifi_cfg->ap.ssid, &ssid_len);
    if (err != ESP_OK)
        return err;
    else if (ssid_len > 32)
        return ESP_ERR_INVALID_SIZE;
    wifi_cfg->ap.ssid_len = (ssid_len > 0) ? (ssid_len - 1) : 0;

    pass_len = 64;
    err = nvs_get_str(app->nvs, WIFIMAN_AP_PASS_KEY, (char *)wifi_cfg->ap.password, &pass_len);
    if (err != ESP_OK)
        return err;
    else if (pass_len > 64)
        return ESP_ERR_INVALID_SIZE;
    if (pass_len > 1)
        wifi_cfg->ap.authmode = WIFI_AUTH_WPA2_PSK;
    else
        wifi_cfg->ap.password[0] = '\0';

    return ESP_OK;
}

esp_err_t _hyWifimanNVSGetSTAConfig(HyWifiman *app, wifi_config_t *wifi_cfg)
{
    if (app == NULL || wifi_cfg == NULL)
        return ESP_ERR_INVALID_ARG;

    size_t ssid_len;
    size_t pass_len;

    wifi_cfg->sta.threshold.authmode = WIFI_AUTH_OPEN;
    wifi_cfg->sta.failure_retry_cnt = 3;

    ssid_len = 32;
    esp_err_t err = nvs_get_str(app->nvs, WIFIMAN_STA_SSID_KEY, (char *)wifi_cfg->sta.ssid, &ssid_len);
    if (err != ESP_OK)
        return err;
    else if (ssid_len > 32)
        return ESP_ERR_INVALID_SIZE;
    if (ssid_len <= 1)
        wifi_cfg->sta.ssid[0] = '\0';

    pass_len = 64;
    err = nvs_get_str(app->nvs, WIFIMAN_STA_PASS_KEY, (char *)wifi_cfg->sta.password, &pass_len);
    if (err != ESP_OK)
        return err;
    else if (pass_len > 64)
        return ESP_ERR_INVALID_SIZE;
    if (pass_len <= 1)
        wifi_cfg->sta.password[0] = '\0';
    else
        wifi_cfg->sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    return ESP_OK;
}

esp_err_t _hyWifimanWiFiInit(HyWifiman *app)
{
    wifi_config_t ap_cfg = {0};
    wifi_config_t sta_cfg = {0};

    esp_err_t err = _hyWifimanNVSGetAPConfig(app, &ap_cfg);
    if (err != ESP_OK)
        return err;

    err = _hyWifimanNVSGetSTAConfig(app, &sta_cfg);
    if (err != ESP_OK)
        return err;

    if (sta_cfg.sta.ssid[0] == '\0')
    {
        err = esp_wifi_set_mode(WIFI_MODE_APSTA);
        if (err != ESP_OK)
            return err;

        err = esp_wifi_set_config(WIFI_IF_AP, &ap_cfg);
        if (err != ESP_OK)
            return err;

        err = esp_wifi_set_config(WIFI_IF_STA, &sta_cfg);
        if (err != ESP_OK)
            return err;

        err = esp_wifi_start();
        if (err != ESP_OK)
            return err;
    }
    else
    {
        err = esp_wifi_set_mode(WIFI_MODE_STA);
        if (err != ESP_OK)
            return err;

        err = esp_wifi_set_config(WIFI_IF_STA, &sta_cfg);
        if (err != ESP_OK)
            return err;

        err = esp_wifi_start();
        if (err != ESP_OK)
            return err;
    }

    return ESP_OK;
}

esp_err_t _hyWifimanWiFiDeinit()
{
    return esp_wifi_stop();
}
