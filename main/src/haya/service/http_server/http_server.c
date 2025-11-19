#include "haya/service/http_server/http_server.h"

esp_err_t hyHttpServerSetup(httpd_handle_t *server, uint16_t port)
{
    httpd_config_t httpd_config = HTTPD_DEFAULT_CONFIG();
    httpd_config.server_port = port;

    esp_err_t err = httpd_start(server, &httpd_config);
    if (err != ESP_OK)
        return err;

    err = httpd_register_err_handler(*server, HTTPD_404_NOT_FOUND, _hyHttpServer404);
    if (err != ESP_OK)
    {
        httpd_stop(*server);
        return err;
    }

    return ESP_OK;
}

esp_err_t _hyHttpServer404(httpd_req_t *req, httpd_err_code_t err)
{
    const size_t len = _404_html_end - _404_html_start;
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, (const char *)_404_html_start, len);
}