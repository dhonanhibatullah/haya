#ifndef DOMAIN_CONTRACTS_NETWORK_INTERFACE_H
#define DOMAIN_CONTRACTS_NETWORK_INTERFACE_H

#include <stdlib.h>

#include "domain/models/error.h"
#include "domain/models/network.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dom_contracts_network_interface_t dom_contracts_network_interface_t;

struct dom_contracts_network_interface_t {
    void* ctx;
    dom_models_error_t (*get_all)(
        dom_contracts_network_interface_t* self,
        dom_models_network_t*              out
    );
    dom_models_error_t (*get_wifi_sta)(
        dom_contracts_network_interface_t* self,
        dom_models_network_interface_t*    out
    );
    dom_models_error_t (*get_ethernet)(
        dom_contracts_network_interface_t* self,
        dom_models_network_interface_t*    out
    );
};

static inline dom_contracts_network_interface_t* dom_contracts_network_interface_new(void* ctx) {
    dom_contracts_network_interface_t* self = (dom_contracts_network_interface_t*)calloc(1, sizeof(dom_contracts_network_interface_t));
    if (!self) {
        return NULL;
    }

    self->ctx = ctx;

    return self;
}

static inline void dom_contracts_network_interface_delete(dom_contracts_network_interface_t* self) {
    if (!self) {
        return;
    }

    self->ctx = NULL;
    free(self);
}

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_CONTRACTS_NETWORK_INTERFACE_H */
