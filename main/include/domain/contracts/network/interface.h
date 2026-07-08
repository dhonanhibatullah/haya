#ifndef DOMAIN_CONTRACTS_NETWORK_INTERFACE_H
#define DOMAIN_CONTRACTS_NETWORK_INTERFACE_H

#include "domain/models/error.h"
#include "domain/models/network.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dom_contracts_network_interface_t dom_contracts_network_interface_t;

struct dom_contracts_network_interface_t {
    void* ctx;
    dom_models_error_t* (*create_wifi_sta)(
        dom_contracts_network_interface_t* self
    );
    dom_models_error_t* (*create_wifi_ap)(
        dom_contracts_network_interface_t* self
    );
    dom_models_error_t* (*create_ethernet)(
        dom_contracts_network_interface_t* self
    );
    dom_models_error_t* (*attach_ethernet)(
        dom_contracts_network_interface_t* self,
        const char*                        if_key,
        void*                              driver_handle
    );
    dom_models_error_t* (*destroy)(
        dom_contracts_network_interface_t* self,
        const char*                        if_key
    );
    dom_models_error_t* (*set_hostname)(
        dom_contracts_network_interface_t* self,
        const char*                        if_key,
        const char*                        hostname
    );
    dom_models_error_t* (*start_dhcp_client)(
        dom_contracts_network_interface_t* self,
        const char*                        if_key
    );
    dom_models_error_t* (*stop_dhcp_client)(
        dom_contracts_network_interface_t* self,
        const char*                        if_key
    );
    dom_models_error_t* (*start_dhcp_server)(
        dom_contracts_network_interface_t* self,
        const char*                        if_key
    );
    dom_models_error_t* (*stop_dhcp_server)(
        dom_contracts_network_interface_t* self,
        const char*                        if_key
    );
    dom_models_error_t* (*get_interface)(
        dom_contracts_network_interface_t* self,
        const char*                        if_key,
        dom_models_network_interface_t*    out
    );
    dom_models_error_t* (*get_interfaces)(
        dom_contracts_network_interface_t* self,
        dom_models_network_t*              out
    );
};

dom_contracts_network_interface_t* dom_contracts_network_interface_new(void* ctx);

void dom_contracts_network_interface_delete(dom_contracts_network_interface_t* self);

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_CONTRACTS_NETWORK_INTERFACE_H */
