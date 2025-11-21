#ifndef __WIFIMAN_TYPES_H
#define __WIFIMAN_TYPES_H

#include <stdbool.h>
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_http_server.h"
#include "nvs_flash.h"
#include "haya/app.h"

typedef struct
{
    httpd_handle_t server;
    nvs_handle_t nvs;

    esp_netif_t *ap_netif;
    esp_netif_t *sta_netif;
    const char *ap_default_ssid;
    const char *ap_default_pass;

    wifi_config_t wifi_cfg_buf;
    esp_event_handler_instance_t wifi_evh;
    esp_event_handler_instance_t ip_evh;

    bool scanning;
    bool connecting;
    bool connected;
    uint8_t reason;
} HyWifiman;

#endif