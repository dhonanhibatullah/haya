#ifndef APPLICATION_NETIF_IMPL_H
#define APPLICATION_NETIF_IMPL_H

#include "application/netif/impl_types.h"
#include "domain/usecases/netif.h"

#ifdef __cplusplus
extern "C" {
#endif

dom_usecases_netif_t* app_netif_impl_new(const app_netif_impl_cfg_t* cfg);

void app_netif_impl_delete(dom_usecases_netif_t* self);

#ifdef __cplusplus
}
#endif

#endif /* APPLICATION_NETIF_IMPL_H */
