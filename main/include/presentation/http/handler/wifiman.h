#ifndef PRESENTATION_HTTP_HANDLER_WIFIMAN_H
#define PRESENTATION_HTTP_HANDLER_WIFIMAN_H

#include "esp_err.h"
#include "esp_http_server.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t pres_http_handler_wifiman_get_status(httpd_req_t* req);

esp_err_t pres_http_handler_wifiman_start_scan(httpd_req_t* req);

esp_err_t pres_http_handler_wifiman_get_scan_result(httpd_req_t* req);

esp_err_t pres_http_handler_wifiman_connect_sta(httpd_req_t* req);

esp_err_t pres_http_handler_wifiman_connect_stored_sta(httpd_req_t* req);

esp_err_t pres_http_handler_wifiman_disconnect_sta(httpd_req_t* req);

esp_err_t pres_http_handler_wifiman_get_stored_sta(httpd_req_t* req);

esp_err_t pres_http_handler_wifiman_set_sta_credential(httpd_req_t* req);

esp_err_t pres_http_handler_wifiman_forget_sta_credential(httpd_req_t* req);

esp_err_t pres_http_handler_wifiman_need_reconnect(httpd_req_t* req);

esp_err_t pres_http_handler_wifiman_try_reconnect(httpd_req_t* req);

#ifdef __cplusplus
}
#endif

#endif /* PRESENTATION_HTTP_HANDLER_WIFIMAN_H */
