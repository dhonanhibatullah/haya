#ifndef PRESENTATION_HTTP_DTO_NETIF_H
#define PRESENTATION_HTTP_DTO_NETIF_H

#include "cJSON.h"
#include "domain/models/network.h"

#ifdef __cplusplus
extern "C" {
#endif

cJSON* pres_http_dto_netif_network_to_json(const dom_models_network_t* network);

cJSON* pres_http_dto_netif_interface_to_json(const dom_models_network_interface_t* interface);

#ifdef __cplusplus
}
#endif

#endif /* PRESENTATION_HTTP_DTO_NETIF_H */
