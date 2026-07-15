#ifndef DOMAIN_CONTRACTS_DEVICE_ETHERNET_H
#define DOMAIN_CONTRACTS_DEVICE_ETHERNET_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "domain/models/error.h"
#include "domain/models/ethernet.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dom_contracts_device_ethernet_t dom_contracts_device_ethernet_t;

struct dom_contracts_device_ethernet_t {
    void* ctx;
    dom_models_error_t (*start)(
        dom_contracts_device_ethernet_t* self
    );
    dom_models_error_t (*stop)(
        dom_contracts_device_ethernet_t* self
    );
    dom_models_error_t (*get_capabilities)(
        dom_contracts_device_ethernet_t*    self,
        dom_models_ethernet_capabilities_t* out
    );
    dom_models_error_t (*get_status)(
        dom_contracts_device_ethernet_t* self,
        dom_models_ethernet_status_t*    out
    );
    dom_models_error_t (*set_mac)(
        dom_contracts_device_ethernet_t* self,
        const uint8_t                    mac[DOM_MODELS_ETHERNET_MAC_LEN]
    );
    dom_models_error_t (*set_link_config)(
        dom_contracts_device_ethernet_t*         self,
        const dom_models_ethernet_link_config_t* config
    );
    dom_models_error_t (*set_promiscuous)(
        dom_contracts_device_ethernet_t* self,
        bool                             enabled
    );
    dom_models_error_t (*set_flow_control)(
        dom_contracts_device_ethernet_t* self,
        bool                             enabled
    );
    dom_models_error_t (*set_phy_loopback)(
        dom_contracts_device_ethernet_t* self,
        bool                             enabled
    );
    dom_models_error_t (*add_event_callback)(
        dom_contracts_device_ethernet_t*     self,
        void*                                cb_ctx,
        dom_models_ethernet_event_callback_t cb_func
    );
    dom_models_error_t (*remove_event_callback)(
        dom_contracts_device_ethernet_t*     self,
        dom_models_ethernet_event_callback_t cb_func
    );
};

static inline dom_contracts_device_ethernet_t* dom_contracts_device_ethernet_new(void* ctx) {
    dom_contracts_device_ethernet_t* self = (dom_contracts_device_ethernet_t*)calloc(1, sizeof(dom_contracts_device_ethernet_t));
    if (!self) {
        return NULL;
    }

    self->ctx = ctx;

    return self;
}

static inline void dom_contracts_device_ethernet_delete(dom_contracts_device_ethernet_t* self) {
    if (!self) {
        return;
    }

    self->ctx = NULL;
    free(self);
}

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_CONTRACTS_DEVICE_ETHERNET_H */
