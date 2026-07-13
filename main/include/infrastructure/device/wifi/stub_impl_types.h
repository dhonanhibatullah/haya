#ifndef INFRASTRUCTURE_DEVICE_WIFI_STUB_IMPL_TYPES_H
#define INFRASTRUCTURE_DEVICE_WIFI_STUB_IMPL_TYPES_H

#include <stdbool.h>

#include "domain/models/wifi.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* sta_if_key;
    const char* ap_if_key;
    const char* default_ssid;
} inf_device_wifi_stub_impl_cfg_t;

#define INF_DEVICE_WIFI_STUB_IMPL_CFG_DEFAULT() \
    {                                           \
        .sta_if_key   = "WIFI_STA_STUB",        \
        .ap_if_key    = "WIFI_AP_STUB",         \
        .default_ssid = "haya-stub",            \
    }

typedef struct {
    inf_device_wifi_stub_impl_cfg_t cfg;
    bool                            initialized;
    bool                            started;
    bool                            connected;
    bool                            ap_started;
    dom_models_wifi_mode_t          mode;
    dom_models_wifi_ap_record_t     connected_ap;
    dom_models_wifi_scan_result_t   scanned;
} inf_device_wifi_stub_impl_ctx_t;

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_DEVICE_WIFI_STUB_IMPL_TYPES_H */
