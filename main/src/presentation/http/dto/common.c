#include "presentation/http/dto/common.h"

#include <stdio.h>
#include <stdlib.h>

#include "cJSON.h"
#include "domain/models/error.h"
#include "esp_err.h"
#include "esp_http_server.h"

dom_models_error_t pres_http_dto_common_recv_json(
    httpd_req_t* req,
    cJSON**      out
) {
    if (!req || !out) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    *out = NULL;

    if (req->content_len == 0) {
        return DOMAIN_MODELS_ERROR_OK;
    }
    if (req->content_len > PRES_HTTP_DTO_COMMON_MAX_BODY_LEN) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    char* body = (char*)calloc(1, req->content_len + 1);
    if (!body) {
        return DOMAIN_MODELS_ERROR_MALLOC_FAILED;
    }

    size_t received = 0;
    while (received < req->content_len) {
        int ret = httpd_req_recv(req, body + received, req->content_len - received);
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            free(body);
            return DOMAIN_MODELS_ERROR_TIMEOUT;
        }
        if (ret <= 0) {
            free(body);
            return DOMAIN_MODELS_ERROR_FAILURE;
        }

        received += (size_t)ret;
    }

    cJSON* json = cJSON_ParseWithLength(body, received);
    free(body);
    if (!json) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    *out = json;

    return DOMAIN_MODELS_ERROR_OK;
}

esp_err_t pres_http_dto_common_send_json(
    httpd_req_t* req,
    cJSON*       json
) {
    if (!req || !json) {
        return ESP_ERR_INVALID_ARG;
    }

    char* body = cJSON_PrintUnformatted(json);
    if (!body) {
        httpd_resp_set_status(req, pres_http_dto_common_status_from_error(DOMAIN_MODELS_ERROR_MALLOC_FAILED));
        httpd_resp_set_type(req, "application/json");
        return httpd_resp_send(req, "{\"error\":\"MALLOC_FAILED\"}", HTTPD_RESP_USE_STRLEN);
    }

    httpd_resp_set_type(req, "application/json");
    esp_err_t err = httpd_resp_send(req, body, HTTPD_RESP_USE_STRLEN);
    cJSON_free(body);

    return err;
}

esp_err_t pres_http_dto_common_send_domain_error(
    httpd_req_t*       req,
    dom_models_error_t err
) {
    if (!req) {
        return ESP_ERR_INVALID_ARG;
    }

    char body[64];
    snprintf(body, sizeof(body), "{\"error\":\"%s\"}", dom_models_error_str(err));

    httpd_resp_set_status(req, pres_http_dto_common_status_from_error(err));
    httpd_resp_set_type(req, "application/json");

    return httpd_resp_send(req, body, HTTPD_RESP_USE_STRLEN);
}

const char* pres_http_dto_common_status_from_error(dom_models_error_t err) {
    switch (err) {
        case DOMAIN_MODELS_ERROR_BAD_ARGUMENT:
            return "400 Bad Request";
        case DOMAIN_MODELS_ERROR_NOT_FOUND:
            return "404 Not Found";
        case DOMAIN_MODELS_ERROR_BAD_STATE:
            return "409 Conflict";
        case DOMAIN_MODELS_ERROR_TIMEOUT:
            return "408 Request Timeout";
        case DOMAIN_MODELS_ERROR_UNIMPLEMENTED:
        case DOMAIN_MODELS_ERROR_NOT_SUPPORTED:
            return "501 Not Implemented";
        case DOMAIN_MODELS_ERROR_OK:
            return "200 OK";
        case DOMAIN_MODELS_ERROR_MALLOC_FAILED:
        case DOMAIN_MODELS_ERROR_FAILURE:
        case DOMAIN_MODELS_ERROR_UNKNOWN:
        default:
            return "500 Internal Server Error";
    }
}
