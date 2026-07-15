#ifndef INFRASTRUCTURE_DEVICE_WIFI_ESP_WIFI_IMPL_TYPES_H
#define INFRASTRUCTURE_DEVICE_WIFI_ESP_WIFI_IMPL_TYPES_H

#include <stdbool.h>
#include <stddef.h>

#include "domain/models/wifi.h"
#include "esp_event_base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* sta_if_key;
    const char* ap_if_key;
    bool        register_event_handler;
} inf_device_wifi_esp_wifi_impl_cfg_t;

#define INF_DEVICE_WIFI_ESP_WIFI_IMPL_EVENT_CALLBACK_MAX 4

#define INF_DEVICE_WIFI_ESP_WIFI_IMPL_CFG_DEFAULT() \
    {                                               \
        .sta_if_key             = "WIFI_STA_DEF",   \
        .ap_if_key              = "WIFI_AP_DEF",    \
        .register_event_handler = true,             \
    }

typedef struct {
    inf_device_wifi_esp_wifi_impl_cfg_t cfg;
    esp_event_handler_instance_t        wifi_event_handler;
    bool                                initialized;
    bool                                wifi_event_handler_registered;
    bool                                started;
    bool                                sta_connected;
    bool                                ap_started;
    dom_models_wifi_scan_result_t       scanned;
    dom_models_wifi_event_callback_t    event_cb_funcs[INF_DEVICE_WIFI_ESP_WIFI_IMPL_EVENT_CALLBACK_MAX];
    void*                               event_cb_ctxs[INF_DEVICE_WIFI_ESP_WIFI_IMPL_EVENT_CALLBACK_MAX];
    size_t                              event_cb_cnt;
} inf_device_wifi_esp_wifi_impl_ctx_t;

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_DEVICE_WIFI_ESP_WIFI_IMPL_TYPES_H */
