#ifndef DOMAIN_MODELS_ETHERNET_H
#define DOMAIN_MODELS_ETHERNET_H

#include <stdbool.h>
#include <stdint.h>

#include "domain/models/network.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DOM_MODELS_ETHERNET_MAC_LEN 6

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
    bool    if_key_available;
    char    if_key[DOM_MODELS_NETWORK_IF_KEY_LEN];
    bool    started;
    bool    link_up;

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
} dom_models_ethernet_status_t;

typedef struct {
    bool                         autoneg;
    dom_models_ethernet_speed_t  speed;
    dom_models_ethernet_duplex_t duplex;
} dom_models_ethernet_link_config_t;

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_MODELS_ETHERNET_H */
