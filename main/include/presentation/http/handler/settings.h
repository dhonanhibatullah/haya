#ifndef PRESENTATION_HTTP_HANDLER_SETTINGS_H
#define PRESENTATION_HTTP_HANDLER_SETTINGS_H

#include "esp_err.h"
#include "esp_http_server.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t pres_http_handler_settings_get_snapshot(httpd_req_t* req);

esp_err_t pres_http_handler_settings_set_preloaded(httpd_req_t* req);

esp_err_t pres_http_handler_settings_get_restart_required(httpd_req_t* req);

esp_err_t pres_http_handler_settings_restart(httpd_req_t* req);

#ifdef __cplusplus
}
#endif

#endif /* PRESENTATION_HTTP_HANDLER_SETTINGS_H */
