#include "presentation/http/handler/wifiman.h"

#include "cJSON.h"
#include "domain/models/error.h"
#include "domain/models/wifi.h"
#include "domain/usecases/wifiman.h"
#include "esp_err.h"
#include "esp_http_server.h"
#include "presentation/http/dto/common.h"
#include "presentation/http/dto/wifiman.h"
#include "presentation/http/handler/wifiman_types.h"

/* Helper Function Prototypes */

static dom_models_error_t get_handler(
    httpd_req_t*                  req,
    pres_http_handler_wifiman_t** out
);

static esp_err_t send_json_and_delete(
    httpd_req_t* req,
    cJSON*       json
);

static esp_err_t send_accepted(httpd_req_t* req);

static esp_err_t send_forgotten(httpd_req_t* req);

/* Handler Implementations */

esp_err_t pres_http_handler_wifiman_get_status(httpd_req_t* req) {
    pres_http_handler_wifiman_t* handler = NULL;
    dom_models_error_t           err     = get_handler(req, &handler);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    dom_usecases_wifiman_status_t status;
    err = handler->wifiman->get_status(handler->wifiman, &status);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    return send_json_and_delete(req, pres_http_dto_wifiman_status_to_json(&status));
}

esp_err_t pres_http_handler_wifiman_start_scan(httpd_req_t* req) {
    pres_http_handler_wifiman_t* handler = NULL;
    cJSON*                       json    = NULL;
    dom_models_error_t           err     = get_handler(req, &handler);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    err = pres_http_dto_common_recv_json(req, &json);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    dom_models_wifi_scan_config_t config;
    err = pres_http_dto_wifiman_parse_scan_config(json, &config);
    if (json) {
        cJSON_Delete(json);
    }
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    err = handler->wifiman->start_scan(handler->wifiman, &config);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    return send_accepted(req);
}

esp_err_t pres_http_handler_wifiman_get_scan_result(httpd_req_t* req) {
    pres_http_handler_wifiman_t* handler = NULL;
    dom_models_error_t           err     = get_handler(req, &handler);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    dom_models_wifi_scan_result_t result;
    err = handler->wifiman->get_scan_result(handler->wifiman, &result);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    return send_json_and_delete(req, pres_http_dto_wifiman_scan_result_to_json(&result));
}

esp_err_t pres_http_handler_wifiman_connect_sta(httpd_req_t* req) {
    pres_http_handler_wifiman_t* handler = NULL;
    cJSON*                       json    = NULL;
    dom_models_error_t           err     = get_handler(req, &handler);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    err = pres_http_dto_common_recv_json(req, &json);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    dom_models_wifi_sta_credential_t credential;
    err = pres_http_dto_wifiman_parse_sta_credential(json, &credential);
    if (json) {
        cJSON_Delete(json);
    }
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    err = handler->wifiman->set_sta_credential(handler->wifiman, &credential);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    esp_err_t http_err = send_accepted(req);
    if (http_err != ESP_OK) {
        return http_err;
    }

    (void)handler->wifiman->connect_stored_sta(handler->wifiman);

    return ESP_OK;
}

esp_err_t pres_http_handler_wifiman_connect_stored_sta(httpd_req_t* req) {
    pres_http_handler_wifiman_t* handler = NULL;
    dom_models_error_t           err     = get_handler(req, &handler);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    dom_usecases_wifiman_stored_sta_t stored_sta;
    err = handler->wifiman->get_stored_sta(handler->wifiman, &stored_sta);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }
    if (!stored_sta.available) {
        return pres_http_dto_common_send_domain_error(req, DOMAIN_MODELS_ERROR_NOT_FOUND);
    }

    esp_err_t http_err = send_accepted(req);
    if (http_err != ESP_OK) {
        return http_err;
    }

    (void)handler->wifiman->connect_stored_sta(handler->wifiman);

    return ESP_OK;
}

esp_err_t pres_http_handler_wifiman_disconnect_sta(httpd_req_t* req) {
    pres_http_handler_wifiman_t* handler = NULL;
    dom_models_error_t           err     = get_handler(req, &handler);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    esp_err_t http_err = send_accepted(req);
    if (http_err != ESP_OK) {
        return http_err;
    }

    (void)handler->wifiman->disconnect_sta(handler->wifiman);

    return ESP_OK;
}

esp_err_t pres_http_handler_wifiman_commit_sta_connection(httpd_req_t* req) {
    pres_http_handler_wifiman_t* handler = NULL;
    dom_models_error_t           err     = get_handler(req, &handler);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    dom_usecases_wifiman_status_t status;
    err = handler->wifiman->get_status(handler->wifiman, &status);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }
    if (!status.wifi.connected) {
        return pres_http_dto_common_send_domain_error(req, DOMAIN_MODELS_ERROR_BAD_STATE);
    }

    esp_err_t http_err = send_accepted(req);
    if (http_err != ESP_OK) {
        return http_err;
    }

    (void)handler->wifiman->commit_sta_connection(handler->wifiman);

    return ESP_OK;
}

esp_err_t pres_http_handler_wifiman_get_stored_sta(httpd_req_t* req) {
    pres_http_handler_wifiman_t* handler = NULL;
    dom_models_error_t           err     = get_handler(req, &handler);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    dom_usecases_wifiman_stored_sta_t stored_sta;
    err = handler->wifiman->get_stored_sta(handler->wifiman, &stored_sta);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    return send_json_and_delete(req, pres_http_dto_wifiman_stored_sta_to_json(&stored_sta));
}

esp_err_t pres_http_handler_wifiman_set_sta_credential(httpd_req_t* req) {
    pres_http_handler_wifiman_t* handler = NULL;
    cJSON*                       json    = NULL;
    dom_models_error_t           err     = get_handler(req, &handler);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    err = pres_http_dto_common_recv_json(req, &json);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    dom_models_wifi_sta_credential_t credential;
    err = pres_http_dto_wifiman_parse_sta_credential(json, &credential);
    if (json) {
        cJSON_Delete(json);
    }
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    err = handler->wifiman->set_sta_credential(handler->wifiman, &credential);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    dom_usecases_wifiman_stored_sta_t stored_sta;
    err = handler->wifiman->get_stored_sta(handler->wifiman, &stored_sta);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    return send_json_and_delete(req, pres_http_dto_wifiman_stored_sta_to_json(&stored_sta));
}

esp_err_t pres_http_handler_wifiman_forget_sta_credential(httpd_req_t* req) {
    pres_http_handler_wifiman_t* handler = NULL;
    dom_models_error_t           err     = get_handler(req, &handler);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    err = handler->wifiman->forget_sta_credential(handler->wifiman);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    return send_forgotten(req);
}

esp_err_t pres_http_handler_wifiman_need_reconnect(httpd_req_t* req) {
    pres_http_handler_wifiman_t* handler = NULL;
    dom_models_error_t           err     = get_handler(req, &handler);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    bool needed = false;
    err = handler->wifiman->need_reconnect(handler->wifiman, &needed);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    return send_json_and_delete(req, pres_http_dto_wifiman_reconnect_need_to_json(needed));
}

esp_err_t pres_http_handler_wifiman_try_reconnect(httpd_req_t* req) {
    pres_http_handler_wifiman_t* handler = NULL;
    dom_models_error_t           err     = get_handler(req, &handler);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    esp_err_t http_err = send_accepted(req);
    if (http_err != ESP_OK) {
        return http_err;
    }

    bool attempted = false;
    (void)handler->wifiman->try_reconnect(handler->wifiman, &attempted);

    return ESP_OK;
}

/* Helper Function Implementations */

static dom_models_error_t get_handler(
    httpd_req_t*                  req,
    pres_http_handler_wifiman_t** out
) {
    if (!req || !out) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    pres_http_handler_wifiman_t* handler = (pres_http_handler_wifiman_t*)req->user_ctx;
    if (!handler || !handler->wifiman) {
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

static esp_err_t send_accepted(httpd_req_t* req) {
    httpd_resp_set_status(req, "202 Accepted");
    return send_json_and_delete(req, pres_http_dto_wifiman_accepted_to_json());
}

static esp_err_t send_forgotten(httpd_req_t* req) {
    return send_json_and_delete(req, pres_http_dto_wifiman_forgotten_to_json());
}
