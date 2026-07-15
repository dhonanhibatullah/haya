#ifndef PRESENTATION_HTTP_DTO_COMMON_H
#define PRESENTATION_HTTP_DTO_COMMON_H

#include "cJSON.h"
#include "domain/models/error.h"
#include "esp_err.h"
#include "esp_http_server.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PRES_HTTP_DTO_COMMON_MAX_BODY_LEN 1024

dom_models_error_t pres_http_dto_common_recv_json(
    httpd_req_t* req,
    cJSON**      out
);

esp_err_t pres_http_dto_common_send_json(
    httpd_req_t* req,
    cJSON*       json
);

esp_err_t pres_http_dto_common_send_domain_error(
    httpd_req_t*       req,
    dom_models_error_t err
);

const char* pres_http_dto_common_status_from_error(dom_models_error_t err);

#ifdef __cplusplus
}
#endif

#endif /* PRESENTATION_HTTP_DTO_COMMON_H */
