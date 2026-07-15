#ifndef PRESENTATION_HTTP_HANDLER_NETIF_TYPES_H
#define PRESENTATION_HTTP_HANDLER_NETIF_TYPES_H

#include "domain/usecases/netif.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    dom_usecases_netif_t* netif;
} pres_http_handler_netif_t;

#ifdef __cplusplus
}
#endif

#endif /* PRESENTATION_HTTP_HANDLER_NETIF_TYPES_H */
