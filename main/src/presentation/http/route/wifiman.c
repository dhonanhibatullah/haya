#include "presentation/http/route/wifiman.h"

#include <stddef.h>

#include "esp_err.h"
#include "esp_http_server.h"
#include "presentation/http/handler/wifiman.h"
#include "presentation/http/handler/wifiman_types.h"

typedef struct {
    const char* uri;
    httpd_method_t method;
    esp_err_t (*handler)(httpd_req_t* req);
} pres_http_route_wifiman_route_t;

static const pres_http_route_wifiman_route_t routes[] = {
    {
        .uri     = "/api/wifi/status",
        .method  = HTTP_GET,
        .handler = pres_http_handler_wifiman_get_status,
    },
    {
        .uri     = "/api/wifi/scan",
        .method  = HTTP_POST,
        .handler = pres_http_handler_wifiman_start_scan,
    },
    {
        .uri     = "/api/wifi/scan",
        .method  = HTTP_GET,
        .handler = pres_http_handler_wifiman_get_scan_result,
    },
    {
        .uri     = "/api/wifi/sta",
        .method  = HTTP_POST,
        .handler = pres_http_handler_wifiman_connect_sta,
    },
    {
        .uri     = "/api/wifi/sta/stored",
        .method  = HTTP_POST,
        .handler = pres_http_handler_wifiman_connect_stored_sta,
    },
    {
        .uri     = "/api/wifi/sta",
        .method  = HTTP_DELETE,
        .handler = pres_http_handler_wifiman_disconnect_sta,
    },
    {
        .uri     = "/api/wifi/sta/credential",
        .method  = HTTP_GET,
        .handler = pres_http_handler_wifiman_get_stored_sta,
    },
    {
        .uri     = "/api/wifi/sta/credential",
        .method  = HTTP_POST,
        .handler = pres_http_handler_wifiman_set_sta_credential,
    },
    {
        .uri     = "/api/wifi/sta/credential",
        .method  = HTTP_DELETE,
        .handler = pres_http_handler_wifiman_forget_sta_credential,
    },
    {
        .uri     = "/api/wifi/reconnect/need",
        .method  = HTTP_GET,
        .handler = pres_http_handler_wifiman_need_reconnect,
    },
    {
        .uri     = "/api/wifi/reconnect",
        .method  = HTTP_POST,
        .handler = pres_http_handler_wifiman_try_reconnect,
    },
};

esp_err_t pres_http_route_wifiman_register(
    httpd_handle_t               server,
    pres_http_handler_wifiman_t* handler
) {
    if (!server || !handler) {
        return ESP_ERR_INVALID_ARG;
    }

    for (size_t i = 0; i < pres_http_route_wifiman_route_cnt(); i++) {
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

esp_err_t pres_http_route_wifiman_unregister(httpd_handle_t server) {
    if (!server) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t result = ESP_OK;

    for (size_t i = 0; i < pres_http_route_wifiman_route_cnt(); i++) {
        esp_err_t err = httpd_unregister_uri_handler(server, routes[i].uri, routes[i].method);
        if (err != ESP_OK && result == ESP_OK) {
            result = err;
        }
    }

    return result;
}

size_t pres_http_route_wifiman_route_cnt(void) {
    return sizeof(routes) / sizeof(routes[0]);
}
