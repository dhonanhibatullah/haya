#ifndef INFRASTRUCTURE_NETWORK_INTERFACE_STUB_IMPL_H
#define INFRASTRUCTURE_NETWORK_INTERFACE_STUB_IMPL_H

#include <stdbool.h>

#include "domain/contracts/network/interface.h"
#include "domain/models/network.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* sta_if_key;
    const char* ap_if_key;
    const char* eth_if_key;
} inf_network_interface_stub_impl_cfg_t;

#define INF_NETWORK_INTERFACE_STUB_IMPL_CFG_DEFAULT() \
    {                                                 \
        .sta_if_key = "WIFI_STA_STUB",                \
        .ap_if_key  = "WIFI_AP_STUB",                 \
        .eth_if_key = "ETH_STUB",                     \
    }

typedef struct {
    bool                           used;
    bool                           attached;
    dom_models_network_interface_t data;
} inf_network_interface_stub_impl_entry_t;

typedef struct {
    inf_network_interface_stub_impl_cfg_t   cfg;
    bool                                    initialized;
    inf_network_interface_stub_impl_entry_t entries[DOM_MODELS_NETWORK_MAX_INTERFACES];
} inf_network_interface_stub_impl_ctx_t;

dom_contracts_network_interface_t* inf_network_interface_stub_impl_new(const inf_network_interface_stub_impl_cfg_t* cfg);

void inf_network_interface_stub_impl_delete(dom_contracts_network_interface_t* self);

dom_models_error_t* inf_network_interface_stub_impl_init(dom_contracts_network_interface_t* self);

dom_models_error_t* inf_network_interface_stub_impl_deinit(dom_contracts_network_interface_t* self);

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_NETWORK_INTERFACE_STUB_IMPL_H */
