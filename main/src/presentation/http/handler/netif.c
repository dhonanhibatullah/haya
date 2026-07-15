#include "presentation/http/handler/netif.h"

#include "cJSON.h"
#include "domain/models/error.h"
#include "domain/models/network.h"
#include "domain/usecases/netif.h"
#include "esp_err.h"
#include "esp_http_server.h"
#include "presentation/http/dto/common.h"
#include "presentation/http/dto/netif.h"
#include "presentation/http/handler/netif_types.h"

/* Helper Function Prototypes */

static dom_models_error_t get_handler(
    httpd_req_t*                req,
    pres_http_handler_netif_t** out
);

static esp_err_t send_json_and_delete(
    httpd_req_t* req,
    cJSON*       json
);

/* Handler Implementations */

esp_err_t pres_http_handler_netif_get_all(httpd_req_t* req) {
    pres_http_handler_netif_t* handler = NULL;
    dom_models_error_t         err     = get_handler(req, &handler);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }
    if (!handler->netif->get_all) {
        return pres_http_dto_common_send_domain_error(req, DOMAIN_MODELS_ERROR_BAD_ARGUMENT);
    }

    dom_models_network_t network;
    err = handler->netif->get_all(handler->netif, &network);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    return send_json_and_delete(req, pres_http_dto_netif_network_to_json(&network));
}

esp_err_t pres_http_handler_netif_get_wifi_sta(httpd_req_t* req) {
    pres_http_handler_netif_t* handler = NULL;
    dom_models_error_t         err     = get_handler(req, &handler);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }
    if (!handler->netif->get_wifi_sta) {
        return pres_http_dto_common_send_domain_error(req, DOMAIN_MODELS_ERROR_BAD_ARGUMENT);
    }

    dom_models_network_interface_t interface;
    err = handler->netif->get_wifi_sta(handler->netif, &interface);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    return send_json_and_delete(req, pres_http_dto_netif_interface_to_json(&interface));
}

esp_err_t pres_http_handler_netif_get_ethernet(httpd_req_t* req) {
    pres_http_handler_netif_t* handler = NULL;
    dom_models_error_t         err     = get_handler(req, &handler);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }
    if (!handler->netif->get_ethernet) {
        return pres_http_dto_common_send_domain_error(req, DOMAIN_MODELS_ERROR_BAD_ARGUMENT);
    }

    dom_models_network_interface_t interface;
    err = handler->netif->get_ethernet(handler->netif, &interface);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    return send_json_and_delete(req, pres_http_dto_netif_interface_to_json(&interface));
}

/* Helper Function Implementations */

static dom_models_error_t get_handler(
    httpd_req_t*                req,
    pres_http_handler_netif_t** out
) {
    if (!req || !out) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    pres_http_handler_netif_t* handler = (pres_http_handler_netif_t*)req->user_ctx;
    if (!handler || !handler->netif) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    *out = handler;

    return DOMAIN_MODELS_ERROR_OK;
}

static esp_err_t send_json_and_delete(
    httpd_req_t* req,
    cJSON*       json
) {
    if (!json) {
        return pres_http_dto_common_send_domain_error(req, DOMAIN_MODELS_ERROR_MALLOC_FAILED);
    }

    esp_err_t err = pres_http_dto_common_send_json(req, json);
    cJSON_Delete(json);

    return err;
}
