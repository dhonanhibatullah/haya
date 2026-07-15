#ifndef DOMAIN_USECASES_NETIF_H
#define DOMAIN_USECASES_NETIF_H

#include <stdlib.h>

#include "domain/models/error.h"
#include "domain/models/network.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dom_usecases_netif_t dom_usecases_netif_t;

struct dom_usecases_netif_t {
    void* ctx;
    dom_models_error_t (*get_all)(
        dom_usecases_netif_t* self,
        dom_models_network_t* out
    );
    dom_models_error_t (*get_wifi_sta)(
        dom_usecases_netif_t*           self,
        dom_models_network_interface_t* out
    );
    dom_models_error_t (*get_ethernet)(
        dom_usecases_netif_t*           self,
        dom_models_network_interface_t* out
    );
};

static inline dom_usecases_netif_t* dom_usecases_netif_new(void* ctx) {
    dom_usecases_netif_t* self = (dom_usecases_netif_t*)calloc(1, sizeof(dom_usecases_netif_t));
    if (!self) {
        return NULL;
    }

    self->ctx = ctx;

    return self;
}

static inline void dom_usecases_netif_delete(dom_usecases_netif_t* self) {
    if (!self) {
        return;
    }

    self->ctx = NULL;
    free(self);
}

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_USECASES_NETIF_H */
