#ifndef INFRASTRUCTURE_DEVICE_WIFI_STUB_IMPL_UTILS_H
#define INFRASTRUCTURE_DEVICE_WIFI_STUB_IMPL_UTILS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "domain/models/error.h"
#include "domain/models/wifi.h"
#include "infrastructure/device/wifi/stub_impl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

dom_models_error_t inf_device_wifi_stub_impl_bad_argument_error(void);
bool               inf_device_wifi_stub_impl_valid_mode(dom_models_wifi_mode_t mode);
size_t             inf_device_wifi_stub_impl_bounded_strlen(const char* value, size_t max_len);
void               inf_device_wifi_stub_impl_copy_cstr(char* dst, size_t dst_size, const char* src);
void               inf_device_wifi_stub_impl_copy_if_key(bool* available, char* out, size_t out_size, const char* if_key);
void               inf_device_wifi_stub_impl_copy_mac(uint8_t dst[DOM_MODELS_WIFI_MAC_LEN], const uint8_t src[DOM_MODELS_WIFI_MAC_LEN]);
const char*        inf_device_wifi_stub_impl_default_ssid(inf_device_wifi_stub_impl_ctx_t* ctx);
void               inf_device_wifi_stub_impl_fill_ap_record(dom_models_wifi_ap_record_t* out, const char* ssid, const uint8_t bssid[DOM_MODELS_WIFI_MAC_LEN], uint8_t channel, int8_t rssi, dom_models_wifi_auth_mode_t auth_mode);
void               inf_device_wifi_stub_impl_fill_default_connected_ap(inf_device_wifi_stub_impl_ctx_t* ctx, const char* ssid, uint8_t channel, dom_models_wifi_auth_mode_t auth_mode);
void               inf_device_wifi_stub_impl_fill_default_scan(inf_device_wifi_stub_impl_ctx_t* ctx, const char* ssid, uint8_t channel);
void               inf_device_wifi_stub_impl_reset_runtime(inf_device_wifi_stub_impl_ctx_t* ctx);

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_DEVICE_WIFI_STUB_IMPL_UTILS_H */
