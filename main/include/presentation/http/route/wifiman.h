#ifndef PRESENTATION_HTTP_ROUTE_WIFIMAN_H
#define PRESENTATION_HTTP_ROUTE_WIFIMAN_H

#include <stddef.h>

#include "esp_err.h"
#include "esp_http_server.h"
#include "presentation/http/handler/wifiman_types.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t pres_http_route_wifiman_register(
    httpd_handle_t               server,
    pres_http_handler_wifiman_t* handler
);

esp_err_t pres_http_route_wifiman_unregister(httpd_handle_t server);

size_t pres_http_route_wifiman_route_cnt(void);

#ifdef __cplusplus
}
#endif

#endif /* PRESENTATION_HTTP_ROUTE_WIFIMAN_H */
