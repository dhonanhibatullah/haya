#ifndef INFRASTRUCTURE_NETWORK_INTERFACE_ESP_NETIF_IMPL_UTILS_H
#define INFRASTRUCTURE_NETWORK_INTERFACE_ESP_NETIF_IMPL_UTILS_H

#include "domain/models/error.h"
#include "domain/models/network.h"
#include "esp_err.h"
#include "esp_netif.h"

#ifdef __cplusplus
extern "C" {
#endif

dom_models_error_t inf_network_interface_esp_netif_impl_error_from_esp(esp_err_t err);

void inf_network_interface_esp_netif_impl_fill_interface(esp_netif_t* netif, dom_models_network_interface_t* out);

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_NETWORK_INTERFACE_ESP_NETIF_IMPL_UTILS_H */
