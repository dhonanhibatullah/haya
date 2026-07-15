#ifndef INFRASTRUCTURE_DEVICE_ETHERNET_ESP_W5500_IMPL_UTILS_H
#define INFRASTRUCTURE_DEVICE_ETHERNET_ESP_W5500_IMPL_UTILS_H

#include <stdbool.h>
#include <stddef.h>

#include "domain/models/error.h"
#include "domain/models/ethernet.h"
#include "esp_err.h"
#include "hal/eth_types.h"

#ifdef __cplusplus
extern "C" {
#endif

dom_models_error_t           inf_device_ethernet_esp_w5500_impl_error_from_esp(esp_err_t err);
size_t                       inf_device_ethernet_esp_w5500_impl_bounded_strlen(const char* value, size_t max_len);
void                         inf_device_ethernet_esp_w5500_impl_copy_cstr(char* dst, size_t dst_size, const char* src);
void                         inf_device_ethernet_esp_w5500_impl_copy_if_key(bool* available, char* out, size_t out_size, const char* if_key);
bool                         inf_device_ethernet_esp_w5500_impl_speed_from_domain(dom_models_ethernet_speed_t in, eth_speed_t* out);
dom_models_ethernet_speed_t  inf_device_ethernet_esp_w5500_impl_speed_to_domain(eth_speed_t in);
bool                         inf_device_ethernet_esp_w5500_impl_duplex_from_domain(dom_models_ethernet_duplex_t in, eth_duplex_t* out);
dom_models_ethernet_duplex_t inf_device_ethernet_esp_w5500_impl_duplex_to_domain(eth_duplex_t in);

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_DEVICE_ETHERNET_ESP_W5500_IMPL_UTILS_H */
