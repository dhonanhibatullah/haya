#ifndef INFRASTRUCTURE_DEVICE_ETHERNET_ESP_W5500_IMPL_H
#define INFRASTRUCTURE_DEVICE_ETHERNET_ESP_W5500_IMPL_H

#include "domain/contracts/device/ethernet.h"
#include "infrastructure/device/ethernet/esp_w5500_impl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

dom_contracts_device_ethernet_t* inf_device_ethernet_esp_w5500_impl_new(const inf_device_ethernet_esp_w5500_impl_cfg_t* cfg);

void inf_device_ethernet_esp_w5500_impl_delete(dom_contracts_device_ethernet_t* self);

dom_models_error_t inf_device_ethernet_esp_w5500_impl_init(dom_contracts_device_ethernet_t* self);

dom_models_error_t inf_device_ethernet_esp_w5500_impl_deinit(dom_contracts_device_ethernet_t* self);

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_DEVICE_ETHERNET_ESP_W5500_IMPL_H */
