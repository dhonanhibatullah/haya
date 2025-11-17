#include "haya/peripheral/wifi.h"

esp_err_t hyPeripheralWiFiSetup(
    esp_netif_t **ap_netif,
    esp_netif_t **sta_netif)
{
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

    *sta_netif = esp_netif_create_default_wifi_sta();
    if (*sta_netif == NULL)
    {
        esp_netif_destroy(*ap_netif);
        *ap_netif = NULL;
        esp_wifi_deinit();
        return ESP_FAIL;
    }

    return ESP_OK;
}