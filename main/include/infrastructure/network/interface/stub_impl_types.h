#ifndef INFRASTRUCTURE_NETWORK_INTERFACE_STUB_IMPL_TYPES_H
#define INFRASTRUCTURE_NETWORK_INTERFACE_STUB_IMPL_TYPES_H

#include "domain/models/network.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char*          sta_if_key;
    dom_models_network_t network;
} inf_network_interface_stub_impl_cfg_t;

#define INF_NETWORK_INTERFACE_STUB_IMPL_CFG_DEFAULT() \
    {                                                \
        .sta_if_key = "WIFI_STA_STUB",               \
    }

typedef struct {
    char                 sta_if_key[DOM_MODELS_NETWORK_IF_KEY_LEN];
    dom_models_network_t network;
} inf_network_interface_stub_impl_ctx_t;

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_NETWORK_INTERFACE_STUB_IMPL_TYPES_H */
