#ifndef APPLICATION_NETIF_IMPL_UTILS_H
#define APPLICATION_NETIF_IMPL_UTILS_H

#include "application/netif/impl_types.h"
#include "domain/models/error.h"

#ifdef __cplusplus
extern "C" {
#endif

dom_models_error_t app_netif_impl_validate_cfg(const app_netif_impl_cfg_t* cfg);

#ifdef __cplusplus
}
#endif

#endif /* APPLICATION_NETIF_IMPL_UTILS_H */
