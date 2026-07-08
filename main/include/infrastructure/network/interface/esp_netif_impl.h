#ifndef INFRASTRUCTURE_NETWORK_INTERFACE_ESP_NETIF_IMPL_H
#define INFRASTRUCTURE_NETWORK_INTERFACE_ESP_NETIF_IMPL_H

#include <stdbool.h>

#include "domain/contracts/network/interface.h"
#include "domain/models/network.h"
#include "esp_eth_netif_glue.h"
#include "esp_netif.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* sta_if_key;
    const char* ap_if_key;
    const char* eth_if_key;
    bool        init_netif;
    bool        create_default_event_loop;
    bool        delete_default_event_loop_on_deinit;
} inf_network_interface_esp_netif_impl_cfg_t;

#define INF_NETWORK_INTERFACE_ESP_NETIF_IMPL_CFG_DEFAULT()     \
    {                                                          \
        .sta_if_key                          = "WIFI_STA_DEF", \
        .ap_if_key                           = "WIFI_AP_DEF",  \
        .eth_if_key                          = "ETH_DEF",      \
        .init_netif                          = true,           \
        .create_default_event_loop           = true,           \
        .delete_default_event_loop_on_deinit = false,          \
    }

typedef enum {
    INF_NETWORK_INTERFACE_ESP_NETIF_IMPL_KIND_NONE = 0,
    INF_NETWORK_INTERFACE_ESP_NETIF_IMPL_KIND_WIFI_STA,
    INF_NETWORK_INTERFACE_ESP_NETIF_IMPL_KIND_WIFI_AP,
    INF_NETWORK_INTERFACE_ESP_NETIF_IMPL_KIND_ETHERNET,
} inf_network_interface_esp_netif_impl_kind_t;

typedef struct {
    bool                                        used;
    char                                        if_key[DOM_MODELS_NETWORK_IF_KEY_LEN];
    esp_netif_t*                                netif;
    esp_eth_netif_glue_handle_t                 eth_glue;
    inf_network_interface_esp_netif_impl_kind_t kind;
} inf_network_interface_esp_netif_impl_entry_t;

typedef struct {
    inf_network_interface_esp_netif_impl_cfg_t   cfg;
    bool                                         netif_initialized;
    bool                                         default_event_loop_created;
    inf_network_interface_esp_netif_impl_entry_t entries[DOM_MODELS_NETWORK_MAX_INTERFACES];
} inf_network_interface_esp_netif_impl_ctx_t;

dom_contracts_network_interface_t* inf_network_interface_esp_netif_impl_new(const inf_network_interface_esp_netif_impl_cfg_t* cfg);

void inf_network_interface_esp_netif_impl_delete(dom_contracts_network_interface_t* self);

dom_models_error_t* inf_network_interface_esp_netif_impl_init(dom_contracts_network_interface_t* self);

dom_models_error_t* inf_network_interface_esp_netif_impl_deinit(dom_contracts_network_interface_t* self);

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_NETWORK_INTERFACE_ESP_NETIF_IMPL_H */
