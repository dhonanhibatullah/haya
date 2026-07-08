#ifndef DOMAIN_CONTRACTS_DEVICE_ETHERNET_H
#define DOMAIN_CONTRACTS_DEVICE_ETHERNET_H

#include <stdbool.h>
#include <stdint.h>

#include "domain/models/error.h"
#include "domain/models/ethernet.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dom_contracts_device_ethernet_t dom_contracts_device_ethernet_t;

struct dom_contracts_device_ethernet_t {
    void* ctx;
    dom_models_error_t* (*start)(
        dom_contracts_device_ethernet_t* self
    );
    dom_models_error_t* (*stop)(
        dom_contracts_device_ethernet_t* self
    );
    dom_models_error_t* (*get_capabilities)(
        dom_contracts_device_ethernet_t*    self,
        dom_models_ethernet_capabilities_t* out
    );
    dom_models_error_t* (*get_status)(
        dom_contracts_device_ethernet_t* self,
        dom_models_ethernet_status_t*    out
    );
    dom_models_error_t* (*set_mac)(
        dom_contracts_device_ethernet_t* self,
        const uint8_t                    mac[DOM_MODELS_ETHERNET_MAC_LEN]
    );
    dom_models_error_t* (*set_link_config)(
        dom_contracts_device_ethernet_t*         self,
        const dom_models_ethernet_link_config_t* config
    );
    dom_models_error_t* (*set_promiscuous)(
        dom_contracts_device_ethernet_t* self,
        bool                             enabled
    );
    dom_models_error_t* (*set_flow_control)(
        dom_contracts_device_ethernet_t* self,
        bool                             enabled
    );
    dom_models_error_t* (*set_phy_loopback)(
        dom_contracts_device_ethernet_t* self,
        bool                             enabled
    );
};

dom_contracts_device_ethernet_t* dom_contracts_device_ethernet_new(void* ctx);

void dom_contracts_device_ethernet_delete(dom_contracts_device_ethernet_t* self);

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_CONTRACTS_DEVICE_ETHERNET_H */
