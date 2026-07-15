#ifndef INFRASTRUCTURE_NETWORK_INTERFACE_ESP_NETIF_IMPL_H
#define INFRASTRUCTURE_NETWORK_INTERFACE_ESP_NETIF_IMPL_H

#include "domain/contracts/network/interface.h"
#include "infrastructure/network/interface/esp_netif_impl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

dom_contracts_network_interface_t* inf_network_interface_esp_netif_impl_new(const inf_network_interface_esp_netif_impl_cfg_t* cfg);

void inf_network_interface_esp_netif_impl_delete(dom_contracts_network_interface_t* self);

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_NETWORK_INTERFACE_ESP_NETIF_IMPL_H */
