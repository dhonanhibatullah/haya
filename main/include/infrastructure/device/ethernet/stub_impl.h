#ifndef INFRASTRUCTURE_DEVICE_ETHERNET_STUB_IMPL_H
#define INFRASTRUCTURE_DEVICE_ETHERNET_STUB_IMPL_H

#include <stdbool.h>
#include <stdint.h>

#include "domain/contracts/device/ethernet.h"
#include "domain/models/ethernet.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* if_key;
    uint8_t     default_mac[DOM_MODELS_ETHERNET_MAC_LEN];
    uint32_t    phy_addr;
} inf_device_ethernet_stub_impl_cfg_t;

#define INF_DEVICE_ETHERNET_STUB_IMPL_CFG_DEFAULT()         \
    {                                                       \
        .if_key      = "ETH_STUB",                          \
        .default_mac = {0x02, 0x00, 0x00, 0x00, 0x30, 0x01}, \
        .phy_addr    = 0,                                   \
    }

typedef struct {
    inf_device_ethernet_stub_impl_cfg_t cfg;
    bool                                initialized;
    bool                                started;
    bool                                link_up;
    uint8_t                             mac[DOM_MODELS_ETHERNET_MAC_LEN];
    dom_models_ethernet_link_config_t   link_config;
    bool                                promiscuous;
    bool                                flow_control;
    bool                                phy_loopback;
} inf_device_ethernet_stub_impl_ctx_t;

dom_contracts_device_ethernet_t* inf_device_ethernet_stub_impl_new(const inf_device_ethernet_stub_impl_cfg_t* cfg);

void inf_device_ethernet_stub_impl_delete(dom_contracts_device_ethernet_t* self);

dom_models_error_t* inf_device_ethernet_stub_impl_init(dom_contracts_device_ethernet_t* self);

dom_models_error_t* inf_device_ethernet_stub_impl_deinit(dom_contracts_device_ethernet_t* self);

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_DEVICE_ETHERNET_STUB_IMPL_H */
