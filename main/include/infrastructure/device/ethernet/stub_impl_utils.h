#ifndef INFRASTRUCTURE_DEVICE_ETHERNET_STUB_IMPL_UTILS_H
#define INFRASTRUCTURE_DEVICE_ETHERNET_STUB_IMPL_UTILS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "domain/models/ethernet.h"

#ifdef __cplusplus
extern "C" {
#endif

size_t inf_device_ethernet_stub_impl_bounded_strlen(const char* value, size_t max_len);
void   inf_device_ethernet_stub_impl_copy_cstr(char* dst, size_t dst_size, const char* src);
void   inf_device_ethernet_stub_impl_copy_if_key(bool* available, char* out, size_t out_size, const char* if_key);
void   inf_device_ethernet_stub_impl_copy_mac(uint8_t dst[DOM_MODELS_ETHERNET_MAC_LEN], const uint8_t src[DOM_MODELS_ETHERNET_MAC_LEN]);
bool   inf_device_ethernet_stub_impl_valid_fixed_link_config(const dom_models_ethernet_link_config_t* config);

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_DEVICE_ETHERNET_STUB_IMPL_UTILS_H */
