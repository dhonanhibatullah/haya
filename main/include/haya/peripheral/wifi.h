#ifndef __HAYA_PERIPHERAL_WIFI_H
#define __HAYA_PERIPHERAL_WIFI_H

#include <string.h>
#include "esp_err.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

esp_err_t hyPeripheralWiFiSetup(
    nvs_handle_t nvs,
    esp_netif_t **ap_netif,
    const char *ap_ssid,
    const char *ap_pass,
    int channel,
    int max_connection);

#endif