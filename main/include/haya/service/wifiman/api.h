#ifndef __WIFIMAN_API_H
#define __WIFIMAN_API_H

#include "esp_err.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "haya/log.h"
#include "haya/app.h"
#include "types.h"
#include "helper.h"
#include "event_handle.h"
#include "http_handle.h"

HyWifiman *hyWifimanNew(
    nvs_handle_t nvs,
    httpd_handle_t server,
    esp_netif_t *ap_netif,
    esp_netif_t *sta_netif,
    const char *ap_default_ssid,
    const char *ap_default_pass);

HyErr hyWifimanStart(HyWifiman *wm);

HyErr hyWifimanStop(HyWifiman *wm);

#endif