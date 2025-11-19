#ifndef __HTTP_SERVER_HANDLE_H
#define __HTTP_SERVER_HANDLE_H

#include "esp_err.h"
#include "esp_http_server.h"

extern const uint8_t _404_html_start[] asm("_binary_404_min_html_start");
extern const uint8_t _404_html_end[] asm("_binary_404_min_html_end");

esp_err_t hyHttpServerSetup(httpd_handle_t server);

esp_err_t _hyHttpServer404(httpd_req_t *req, httpd_err_code_t err);

#endif