#ifndef __WIFIMAN_HTTP_HANDLE_H
#define __WIFIMAN_HTTP_HANDLE_H

#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_http_server.h"
#include "nvs_flash.h"
#include "cJSON.h"
#include "haya/log.h"
#include "types.h"
#include "html.h"

extern httpd_uri_t _hy_wifiman_get_root;
extern httpd_uri_t _hy_wifiman_get_api_scanned;
extern httpd_uri_t _hy_wifiman_get_api_connection;
extern httpd_uri_t _hy_wifiman_post_api_scan;
extern httpd_uri_t _hy_wifiman_post_api_connect;

esp_err_t _hyWifimanHTTPHandleRoute(httpd_handle_t server, void *ctx);

void _hyWifimanHTTPHandleUnroute(httpd_handle_t server);

esp_err_t _hyWifimanGetRoot(httpd_req_t *req);

esp_err_t _hyWifimanGetAPIScanned(httpd_req_t *req);

esp_err_t _hyWifimanGetAPIConnection(httpd_req_t *req);

esp_err_t _hyWifimanPostAPIScan(httpd_req_t *req);

esp_err_t _hyWifimanPostAPIConnect(httpd_req_t *req);

esp_err_t _hyWifimanSendErrorResponse(httpd_req_t *req, uint16_t code, const char *msg);

#endif