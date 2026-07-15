#include "presentation/http/handler/settings.h"

#include "cJSON.h"
#include "domain/models/error.h"
#include "domain/usecases/settings.h"
#include "esp_err.h"
#include "esp_http_server.h"
#include "presentation/http/dto/common.h"
#include "presentation/http/dto/settings.h"
#include "presentation/http/handler/settings_types.h"

/* Helper Function Prototypes */

static dom_models_error_t get_handler(
    httpd_req_t*                   req,
    pres_http_handler_settings_t** out
);

static esp_err_t send_json_and_delete(
    httpd_req_t* req,
    cJSON*       json
);

static esp_err_t send_accepted(httpd_req_t* req);

/* Handler Implementations */

esp_err_t pres_http_handler_settings_get_snapshot(httpd_req_t* req) {
    pres_http_handler_settings_t* handler = NULL;
    dom_models_error_t            err     = get_handler(req, &handler);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    dom_usecases_settings_snapshot_t snapshot;
    err = handler->settings->get_snapshot(handler->settings, &snapshot);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    return send_json_and_delete(req, pres_http_dto_settings_snapshot_to_json(&snapshot));
}

esp_err_t pres_http_handler_settings_set_preloaded(httpd_req_t* req) {
    pres_http_handler_settings_t* handler = NULL;
    cJSON*                        json    = NULL;
    dom_models_error_t            err     = get_handler(req, &handler);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    err = pres_http_dto_common_recv_json(req, &json);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    dom_usecases_settings_preloaded_update_t update;
    err = pres_http_dto_settings_parse_preloaded_update(json, &update);
    if (json) {
        cJSON_Delete(json);
    }
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    bool restart_required = false;
    err = handler->settings->set_preloaded(handler->settings, &update, &restart_required);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    return send_json_and_delete(req, pres_http_dto_settings_preloaded_updated_to_json(restart_required));
}

esp_err_t pres_http_handler_settings_get_restart_required(httpd_req_t* req) {
    pres_http_handler_settings_t* handler = NULL;
    dom_models_error_t            err     = get_handler(req, &handler);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    bool restart_required = false;
    err = handler->settings->get_restart_required(handler->settings, &restart_required);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    return send_json_and_delete(req, pres_http_dto_settings_restart_required_to_json(restart_required));
}

esp_err_t pres_http_handler_settings_restart(httpd_req_t* req) {
    pres_http_handler_settings_t* handler = NULL;
    dom_models_error_t            err     = get_handler(req, &handler);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return pres_http_dto_common_send_domain_error(req, err);
    }

    esp_err_t http_err = send_accepted(req);
    if (http_err != ESP_OK) {
        return http_err;
    }

    (void)handler->settings->restart(handler->settings);

    return ESP_OK;
}

/* Helper Function Implementations */

static dom_models_error_t get_handler(
    httpd_req_t*                   req,
    pres_http_handler_settings_t** out
) {
    if (!req || !out) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    pres_http_handler_settings_t* handler = (pres_http_handler_settings_t*)req->user_ctx;
    if (!handler ||
        !handler->settings ||
        !handler->settings->get_snapshot ||
        !handler->settings->set_preloaded ||
        !handler->settings->get_restart_required ||
        !handler->settings->restart) {
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
    return send_json_and_delete(req, pres_http_dto_settings_accepted_to_json());
}
