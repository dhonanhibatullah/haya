#ifndef APPLICATION_NETIF_IMPL_TYPES_H
#define APPLICATION_NETIF_IMPL_TYPES_H

#include "domain/contracts/logger/leveled.h"
#include "domain/contracts/network/interface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    dom_contracts_logger_leveled_t*    logger;
    dom_contracts_network_interface_t* network_interface;
} app_netif_impl_cfg_t;

typedef struct {
    app_netif_impl_cfg_t cfg;
} app_netif_impl_ctx_t;

#ifdef __cplusplus
}
#endif

#endif /* APPLICATION_NETIF_IMPL_TYPES_H */
