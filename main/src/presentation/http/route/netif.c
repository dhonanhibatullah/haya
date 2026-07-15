#include "presentation/http/route/netif.h"

#include <stddef.h>

#include "esp_err.h"
#include "esp_http_server.h"
#include "presentation/http/handler/netif.h"
#include "presentation/http/handler/netif_types.h"

typedef struct {
    const char* uri;
    httpd_method_t method;
    esp_err_t (*handler)(httpd_req_t* req);
} pres_http_route_netif_route_t;

static const pres_http_route_netif_route_t routes[] = {
    {
        .uri     = "/api/netif",
        .method  = HTTP_GET,
        .handler = pres_http_handler_netif_get_all,
    },
    {
        .uri     = "/api/netif/wifi/sta",
        .method  = HTTP_GET,
        .handler = pres_http_handler_netif_get_wifi_sta,
    },
    {
        .uri     = "/api/netif/ethernet",
        .method  = HTTP_GET,
        .handler = pres_http_handler_netif_get_ethernet,
    },
};

esp_err_t pres_http_route_netif_register(
    httpd_handle_t             server,
    pres_http_handler_netif_t* handler
) {
    if (!server || !handler) {
        return ESP_ERR_INVALID_ARG;
    }

    for (size_t i = 0; i < pres_http_route_netif_route_cnt(); i++) {
        httpd_uri_t route = {
            .uri      = routes[i].uri,
            .method   = routes[i].method,
            .handler  = routes[i].handler,
            .user_ctx = handler,
        };

        esp_err_t err = httpd_register_uri_handler(server, &route);
        if (err != ESP_OK) {
            return err;
        }
    }

    return ESP_OK;
}

esp_err_t pres_http_route_netif_unregister(httpd_handle_t server) {
    if (!server) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t result = ESP_OK;

    for (size_t i = 0; i < pres_http_route_netif_route_cnt(); i++) {
        esp_err_t err = httpd_unregister_uri_handler(server, routes[i].uri, routes[i].method);
        if (err != ESP_OK && result == ESP_OK) {
            result = err;
        }
    }

    return result;
}

size_t pres_http_route_netif_route_cnt(void) {
    return sizeof(routes) / sizeof(routes[0]);
}
