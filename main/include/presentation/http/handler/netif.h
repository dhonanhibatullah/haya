#ifndef PRESENTATION_HTTP_HANDLER_NETIF_H
#define PRESENTATION_HTTP_HANDLER_NETIF_H

#include "esp_err.h"
#include "esp_http_server.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t pres_http_handler_netif_get_all(httpd_req_t* req);

esp_err_t pres_http_handler_netif_get_wifi_sta(httpd_req_t* req);

esp_err_t pres_http_handler_netif_get_ethernet(httpd_req_t* req);

#ifdef __cplusplus
}
#endif

#endif /* PRESENTATION_HTTP_HANDLER_NETIF_H */
