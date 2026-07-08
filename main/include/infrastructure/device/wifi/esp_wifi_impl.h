#ifndef INFRASTRUCTURE_DEVICE_WIFI_ESP_WIFI_IMPL_H
#define INFRASTRUCTURE_DEVICE_WIFI_ESP_WIFI_IMPL_H

#include <stdbool.h>

#include "esp_event.h"

#include "domain/contracts/device/wifi.h"
#include "domain/models/wifi.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* sta_if_key;
    const char* ap_if_key;
    bool        register_event_handler;
} inf_device_wifi_esp_wifi_impl_cfg_t;

#define INF_DEVICE_WIFI_ESP_WIFI_IMPL_CFG_DEFAULT() \
    {                                               \
        .sta_if_key             = "WIFI_STA_DEF",   \
        .ap_if_key              = "WIFI_AP_DEF",    \
        .register_event_handler = true,             \
    }

typedef struct {
    inf_device_wifi_esp_wifi_impl_cfg_t cfg;
    esp_event_handler_instance_t        wifi_event_handler;
    bool                                wifi_initialized;
    bool                                wifi_event_handler_registered;
    bool                                started;
    bool                                sta_connected;
    bool                                ap_started;
    dom_models_wifi_scan_result_t       scanned;
} inf_device_wifi_esp_wifi_impl_ctx_t;

dom_contracts_device_wifi_t* inf_device_wifi_esp_wifi_impl_new(const inf_device_wifi_esp_wifi_impl_cfg_t* cfg);

void inf_device_wifi_esp_wifi_impl_delete(dom_contracts_device_wifi_t* self);

dom_models_error_t* inf_device_wifi_esp_wifi_impl_init(dom_contracts_device_wifi_t* self);

dom_models_error_t* inf_device_wifi_esp_wifi_impl_deinit(dom_contracts_device_wifi_t* self);

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_DEVICE_WIFI_ESP_WIFI_IMPL_H */
