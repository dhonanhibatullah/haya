#ifndef INFRASTRUCTURE_DEVICE_WIFI_ESP_WIFI_IMPL_UTILS_H
#define INFRASTRUCTURE_DEVICE_WIFI_ESP_WIFI_IMPL_UTILS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "domain/models/error.h"
#include "domain/models/wifi.h"
#include "esp_err.h"
#include "esp_wifi_types_generic.h"
#include "infrastructure/device/wifi/esp_wifi_impl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

dom_models_error_t               inf_device_wifi_esp_wifi_impl_error_from_esp(esp_err_t err);
size_t                           inf_device_wifi_esp_wifi_impl_bounded_strlen(const char* value, size_t max_len);
size_t                           inf_device_wifi_esp_wifi_impl_bounded_byte_strlen(const uint8_t* value, size_t max_len);
void                             inf_device_wifi_esp_wifi_impl_copy_cstr(char* dst, size_t dst_size, const char* src);
void                             inf_device_wifi_esp_wifi_impl_copy_cstr_to_bytes(uint8_t* dst, size_t dst_size, const char* src);
void                             inf_device_wifi_esp_wifi_impl_copy_bytes_to_cstr(char* dst, size_t dst_size, const uint8_t* src, size_t src_size);
bool                             inf_device_wifi_esp_wifi_impl_wifi_mode_from_domain(dom_models_wifi_mode_t in, wifi_mode_t* out);
dom_models_wifi_mode_t           inf_device_wifi_esp_wifi_impl_wifi_mode_to_domain(wifi_mode_t in);
bool                             inf_device_wifi_esp_wifi_impl_wifi_auth_from_domain(dom_models_wifi_auth_mode_t in, wifi_auth_mode_t* out);
dom_models_wifi_auth_mode_t      inf_device_wifi_esp_wifi_impl_wifi_auth_to_domain(wifi_auth_mode_t in);
dom_models_wifi_cipher_t         inf_device_wifi_esp_wifi_impl_wifi_cipher_to_domain(wifi_cipher_type_t in);
dom_models_wifi_bandwidth_t      inf_device_wifi_esp_wifi_impl_wifi_bandwidth_to_domain(wifi_bandwidth_t in);
dom_models_wifi_second_channel_t inf_device_wifi_esp_wifi_impl_wifi_second_channel_to_domain(wifi_second_chan_t in);
uint32_t                         inf_device_wifi_esp_wifi_impl_phy_flags_from_ap_record(const wifi_ap_record_t* record);
uint32_t                         inf_device_wifi_esp_wifi_impl_phy_flags_from_sta_info(const wifi_sta_info_t* sta);
void                             inf_device_wifi_esp_wifi_impl_copy_if_key(bool* available, char* out, size_t out_size, const char* if_key);
void                             inf_device_wifi_esp_wifi_impl_copy_ap_record(dom_models_wifi_ap_record_t* out, const wifi_ap_record_t* in);
void                             inf_device_wifi_esp_wifi_impl_copy_ap_client(dom_models_wifi_ap_client_t* out, const wifi_sta_info_t* in);
esp_err_t                        inf_device_wifi_esp_wifi_impl_ensure_sta_mode(void);
esp_err_t                        inf_device_wifi_esp_wifi_impl_ensure_ap_mode(void);
void                             inf_device_wifi_esp_wifi_impl_clear_scan_records(dom_models_wifi_scan_result_t* out);
void                             inf_device_wifi_esp_wifi_impl_load_scan_records(inf_device_wifi_esp_wifi_impl_ctx_t* ctx);

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_DEVICE_WIFI_ESP_WIFI_IMPL_UTILS_H */
