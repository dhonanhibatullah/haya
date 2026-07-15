#ifndef DOMAIN_MODELS_ETHERNET_H
#define DOMAIN_MODELS_ETHERNET_H

#include <stdbool.h>
#include <stdint.h>

#include "domain/models/network.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DOM_MODELS_ETHERNET_MAC_LEN 6

#define DOM_MODELS_ETHERNET_CAPABILITY_SET_MAC      (1u << 0)
#define DOM_MODELS_ETHERNET_CAPABILITY_LINK_CONFIG  (1u << 1)
#define DOM_MODELS_ETHERNET_CAPABILITY_PROMISCUOUS  (1u << 2)
#define DOM_MODELS_ETHERNET_CAPABILITY_FLOW_CONTROL (1u << 3)
#define DOM_MODELS_ETHERNET_CAPABILITY_PHY_LOOPBACK (1u << 4)

typedef enum {
    DOM_MODELS_ETHERNET_SPEED_UNKNOWN = 0,
    DOM_MODELS_ETHERNET_SPEED_10M,
    DOM_MODELS_ETHERNET_SPEED_100M,
    DOM_MODELS_ETHERNET_SPEED_1000M,
} dom_models_ethernet_speed_t;

typedef enum {
    DOM_MODELS_ETHERNET_DUPLEX_UNKNOWN = 0,
    DOM_MODELS_ETHERNET_DUPLEX_HALF,
    DOM_MODELS_ETHERNET_DUPLEX_FULL,
} dom_models_ethernet_duplex_t;

typedef struct {
    uint32_t flags;
} dom_models_ethernet_capabilities_t;

typedef struct {
    bool if_key_available;
    char if_key[DOM_MODELS_NETWORK_IF_KEY_LEN];
    bool started;
    bool link_up;

    bool    mac_available;
    uint8_t mac[DOM_MODELS_ETHERNET_MAC_LEN];

    bool     phy_addr_available;
    uint32_t phy_addr;

    bool autoneg_available;
    bool autoneg;

    bool                        speed_available;
    dom_models_ethernet_speed_t speed;

    bool                         duplex_available;
    dom_models_ethernet_duplex_t duplex;

    bool promiscuous_available;
    bool promiscuous;

    bool flow_control_available;
    bool flow_control;

    bool phy_loopback_available;
    bool phy_loopback;
} dom_models_ethernet_status_t;

/* If autoneg is true, speed and duplex are ignored. If false, both must be non-UNKNOWN. */
typedef struct {
    bool                         autoneg;
    dom_models_ethernet_speed_t  speed;
    dom_models_ethernet_duplex_t duplex;
} dom_models_ethernet_link_config_t;

typedef enum {
    DOM_MODELS_ETHERNET_EVENT_UNKNOWN = 0,
    DOM_MODELS_ETHERNET_EVENT_STARTED,
    DOM_MODELS_ETHERNET_EVENT_STOPPED,
    DOM_MODELS_ETHERNET_EVENT_LINK_UP,
    DOM_MODELS_ETHERNET_EVENT_LINK_DOWN,
    DOM_MODELS_ETHERNET_EVENT_GOT_IP,
    DOM_MODELS_ETHERNET_EVENT_LOST_IP,
} dom_models_ethernet_event_type_t;

typedef struct {
    dom_models_ethernet_event_type_t type;
    uint32_t                         driver_status;
} dom_models_ethernet_event_t;

typedef void (*dom_models_ethernet_event_callback_t)(void* cb_ctx, const dom_models_ethernet_event_t* event);

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_MODELS_ETHERNET_H */
