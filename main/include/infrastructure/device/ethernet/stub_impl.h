#ifndef INFRASTRUCTURE_DEVICE_ETHERNET_STUB_IMPL_H
#define INFRASTRUCTURE_DEVICE_ETHERNET_STUB_IMPL_H

#include "domain/contracts/device/ethernet.h"
#include "infrastructure/device/ethernet/stub_impl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

dom_contracts_device_ethernet_t* inf_device_ethernet_stub_impl_new(const inf_device_ethernet_stub_impl_cfg_t* cfg);

void inf_device_ethernet_stub_impl_delete(dom_contracts_device_ethernet_t* self);

dom_models_error_t inf_device_ethernet_stub_impl_init(dom_contracts_device_ethernet_t* self);

dom_models_error_t inf_device_ethernet_stub_impl_deinit(dom_contracts_device_ethernet_t* self);

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_DEVICE_ETHERNET_STUB_IMPL_H */
