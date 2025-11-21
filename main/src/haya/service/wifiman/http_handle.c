#include "haya/service/wifiman/http_handle.h"

const char *WIFIMAN_HTTP_TAG = "wifiman/http";

httpd_uri_t _hy_wifiman_get_root = {
    .uri = "/wifiman",
    .method = HTTP_GET,
    .handler = _hyWifimanGetRoot,
    .user_ctx = NULL,
};

httpd_uri_t _hy_wifiman_get_api_scanned = {
    .uri = "/wifiman/api/scanned",
    .method = HTTP_GET,
    .handler = _hyWifimanGetAPIScanned,
    .user_ctx = NULL,
};

httpd_uri_t _hy_wifiman_get_api_status = {
    .uri = "/wifiman/api/status",
    .method = HTTP_GET,
    .handler = _hyWifimanGetAPIStatus,
    .user_ctx = NULL,
};

httpd_uri_t _hy_wifiman_get_api_connsts = {
    .uri = "/wifiman/api/connsts",
    .method = HTTP_GET,
    .handler = _hyWifimanGetAPIConnsts,
    .user_ctx = NULL,
};

httpd_uri_t _hy_wifiman_post_api_scan = {
    .uri = "/wifiman/api/scan",
    .method = HTTP_POST,
    .handler = _hyWifimanPostAPIScan,
    .user_ctx = NULL,
};

httpd_uri_t _hy_wifiman_post_api_connect = {
    .uri = "/wifiman/api/connect",
    .method = HTTP_POST,
    .handler = _hyWifimanPostAPIConnect,
    .user_ctx = NULL,
};

httpd_uri_t _hy_wifiman_post_api_commit = {
    .uri = "/wifiman/api/commit",
    .method = HTTP_POST,
    .handler = _hyWifimanPostAPICommit,
    .user_ctx = NULL,
};

esp_err_t _hyWifimanHTTPHandleRoute(httpd_handle_t server, void *ctx)
{
    _hy_wifiman_get_root.user_ctx = ctx;
    _hy_wifiman_get_api_scanned.user_ctx = ctx;
    _hy_wifiman_get_api_status.user_ctx = ctx;
    _hy_wifiman_get_api_connsts.user_ctx = ctx;
    _hy_wifiman_post_api_scan.user_ctx = ctx;
    _hy_wifiman_post_api_connect.user_ctx = ctx;
    _hy_wifiman_post_api_commit.user_ctx = ctx;

    esp_err_t err = httpd_register_uri_handler(server, &_hy_wifiman_get_root);
    if (err != ESP_OK)
    {
        _hyWifimanHTTPHandleUnroute(server);
        return err;
    }

    err = httpd_register_uri_handler(server, &_hy_wifiman_get_api_scanned);
    if (err != ESP_OK)
    {
        _hyWifimanHTTPHandleUnroute(server);
        return err;
    }

    err = httpd_register_uri_handler(server, &_hy_wifiman_get_api_status);
    if (err != ESP_OK)
    {
        _hyWifimanHTTPHandleUnroute(server);
        return err;
    }

    err = httpd_register_uri_handler(server, &_hy_wifiman_get_api_connsts);
    if (err != ESP_OK)
    {
        _hyWifimanHTTPHandleUnroute(server);
        return err;
    }

    err = httpd_register_uri_handler(server, &_hy_wifiman_post_api_scan);
    if (err != ESP_OK)
    {
        _hyWifimanHTTPHandleUnroute(server);
        return err;
    }

    err = httpd_register_uri_handler(server, &_hy_wifiman_post_api_connect);
    if (err != ESP_OK)
    {
        _hyWifimanHTTPHandleUnroute(server);
        return err;
    }

    err = httpd_register_uri_handler(server, &_hy_wifiman_post_api_commit);
    if (err != ESP_OK)
    {
        _hyWifimanHTTPHandleUnroute(server);
        return err;
    }

    return ESP_OK;
}

void _hyWifimanHTTPHandleUnroute(httpd_handle_t server)
{
    httpd_unregister_uri_handler(server, _hy_wifiman_get_root.uri, _hy_wifiman_get_root.method);
    httpd_unregister_uri_handler(server, _hy_wifiman_get_api_scanned.uri, _hy_wifiman_get_api_scanned.method);
    httpd_unregister_uri_handler(server, _hy_wifiman_get_api_status.uri, _hy_wifiman_get_api_status.method);
    httpd_unregister_uri_handler(server, _hy_wifiman_get_api_connsts.uri, _hy_wifiman_get_api_connsts.method);
    httpd_unregister_uri_handler(server, _hy_wifiman_post_api_scan.uri, _hy_wifiman_post_api_scan.method);
    httpd_unregister_uri_handler(server, _hy_wifiman_post_api_connect.uri, _hy_wifiman_post_api_connect.method);
    httpd_unregister_uri_handler(server, _hy_wifiman_post_api_commit.uri, _hy_wifiman_post_api_commit.method);
}

esp_err_t _hyWifimanGetRoot(httpd_req_t *req)
{
    const size_t index_html_len = index_html_end - index_html_start;
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, (const char *)index_html_start, index_html_len);
}

esp_err_t _hyWifimanGetAPIScanned(httpd_req_t *req)
{
    HyWifiman *app = (HyWifiman *)req->user_ctx;
    uint16_t ap_count = 0;
    esp_err_t err;

    if (app->scanning)
    {
        hyLogWarn(
            WIFIMAN_HTTP_TAG,
            "%s: 409, scan is in progress",
            _hy_wifiman_get_api_scanned.uri);
        return _hyWifimanSendErrorResponse(req, 409, "Scan is in progress");
    }

    esp_wifi_scan_get_ap_num(&ap_count);
    if (ap_count == 0)
    {
        httpd_resp_set_type(req, "application/json");
        return httpd_resp_send(req, "[]", 2);
    }

    wifi_ap_record_t *ap_info = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * ap_count);
    if (ap_info == NULL)
    {
        hyLogError(
            WIFIMAN_HTTP_TAG,
            "%s: 500, malloc failed",
            _hy_wifiman_get_api_scanned.uri);
        return _hyWifimanSendErrorResponse(req, 500, "Memory allocation failed");
    }

    err = esp_wifi_scan_get_ap_records(&ap_count, ap_info);
    if (err != ESP_OK)
    {
        free(ap_info);
        hyLogError(
            WIFIMAN_HTTP_TAG,
            "%s: 500, failed to get scanned AP records: %s",
            _hy_wifiman_get_api_scanned.uri,
            esp_err_to_name(err));
        return _hyWifimanSendErrorResponse(req, 500, "AP scan records retrieval failed");
    }

    cJSON *root = cJSON_CreateArray();
    for (int i = 0; i < ap_count; i++)
    {
        cJSON *item = cJSON_CreateObject();
        cJSON_AddStringToObject(item, "ssid", (char *)ap_info[i].ssid);
        cJSON_AddNumberToObject(item, "rssi", ap_info[i].rssi);
        cJSON_AddNumberToObject(item, "auth", ap_info[i].authmode);
        cJSON_AddNumberToObject(item, "channel", ap_info[i].primary);
        cJSON_AddItemToArray(root, item);
    }

    char *json_str = cJSON_PrintUnformatted(root);
    httpd_resp_set_type(req, "application/json");
    err = httpd_resp_send(req, json_str, strlen(json_str));

    free(json_str);
    cJSON_Delete(root);
    free(ap_info);

    return err;
}

esp_err_t _hyWifimanGetAPIStatus(httpd_req_t *req)
{
    HyWifiman *app = (HyWifiman *)req->user_ctx;

    char ip_str[16] = "0.0.0.0";
    if (app->connected && app->sta_netif)
    {
        esp_netif_ip_info_t ip_info;
        if (esp_netif_get_ip_info(app->sta_netif, &ip_info) == ESP_OK)
            snprintf(ip_str, sizeof(ip_str), IPSTR, IP2STR(&ip_info.ip));
    }

    char json_buf[128];
    int len = snprintf(
        json_buf, sizeof(json_buf),
        "{\"connected\":%s,\"ssid\":\"%s\",\"ip\":\"%s\"}",
        app->connected ? "true" : "false",
        app->connected ? (char *)app->wifi_cfg_buf.sta.ssid : "",
        ip_str);

    if (len >= sizeof(json_buf))
        len = sizeof(json_buf) - 1;

    hyLogInfo(
        WIFIMAN_HTTP_TAG,
        "%s: 200, status sent",
        _hy_wifiman_get_api_status.uri);

    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, json_buf, len);
}

esp_err_t _hyWifimanGetAPIConnsts(httpd_req_t *req)
{
    HyWifiman *app = (HyWifiman *)req->user_ctx;

    bool is_done = !app->connecting;

    char json_buf[128];
    int len = snprintf(
        json_buf, sizeof(json_buf),
        "{\"done\":%s,\"success\":%s,\"reason\":%u}",
        is_done ? "true" : "false",
        app->connected ? "true" : "false",
        app->reason);

    if (len >= sizeof(json_buf))
        len = sizeof(json_buf) - 1;

    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, json_buf, len);
}

esp_err_t _hyWifimanPostAPIScan(httpd_req_t *req)
{
    HyWifiman *app = (HyWifiman *)req->user_ctx;

    if (app->scanning)
    {
        hyLogWarn(
            WIFIMAN_HTTP_TAG,
            "%s: 409, scan already in progress",
            _hy_wifiman_post_api_scan.uri);
        return _hyWifimanSendErrorResponse(req, 409, "Scan already in progress");
    }

    app->scanning = true;
    esp_err_t err = esp_wifi_scan_start(NULL, false);
    if (err != ESP_OK)
    {
        app->scanning = false;
        hyLogError(
            WIFIMAN_HTTP_TAG,
            "%s: 500, failed to start scan: %s",
            _hy_wifiman_post_api_scan.uri,
            esp_err_to_name(err));
        return _hyWifimanSendErrorResponse(req, 500, "Failed to start scan");
    }
    hyLogInfo(
        WIFIMAN_HTTP_TAG,
        "%s: WiFi scan started",
        _hy_wifiman_post_api_scan.uri);

    return httpd_resp_send(req, "", 0);
}

esp_err_t _hyWifimanPostAPIConnect(httpd_req_t *req)
{
    char buf[256];
    int ret, remaining = req->content_len;
    HyWifiman *app = (HyWifiman *)req->user_ctx;

    if (app->connecting)
    {
        hyLogWarn(
            WIFIMAN_HTTP_TAG,
            "%s: 409, connecting in progress",
            _hy_wifiman_post_api_connect.uri);
        return _hyWifimanSendErrorResponse(req, 409, "Connecting in progress");
    }

    if (remaining >= sizeof(buf))
    {
        hyLogWarn(
            WIFIMAN_HTTP_TAG,
            "%s: 400, payload too large",
            _hy_wifiman_post_api_connect.uri);
        return _hyWifimanSendErrorResponse(req, 400, "Payload too large");
    }

    if ((ret = httpd_req_recv(req, buf, remaining)) <= 0)
    {
        hyLogError(
            WIFIMAN_HTTP_TAG,
            "%s: 500, failed to receive request",
            _hy_wifiman_post_api_connect.uri);
        return _hyWifimanSendErrorResponse(req, 500, "Failed to receive request");
    }
    buf[ret] = '\0';

    cJSON *root = cJSON_Parse(buf);
    if (root == NULL)
    {
        hyLogWarn(
            WIFIMAN_HTTP_TAG,
            "%s: 400, failed to parse JSON",
            _hy_wifiman_post_api_connect.uri);
        return _hyWifimanSendErrorResponse(req, 400, "Invalid JSON");
    }

    cJSON *ssid_item = cJSON_GetObjectItem(root, "ssid");
    cJSON *pass_item = cJSON_GetObjectItem(root, "password");

    if (!cJSON_IsString(ssid_item) || (ssid_item->valuestring == NULL))
    {
        hyLogWarn(
            WIFIMAN_HTTP_TAG,
            "%s: 400, missing SSID",
            _hy_wifiman_post_api_connect.uri);
        cJSON_Delete(root);
        return _hyWifimanSendErrorResponse(req, 400, "Missing SSID");
    }
    memset(&app->wifi_cfg_buf, 0, sizeof(wifi_config_t));
    strlcpy(
        (char *)app->wifi_cfg_buf.sta.ssid,
        ssid_item->valuestring,
        sizeof(app->wifi_cfg_buf.sta.ssid));

    if (cJSON_IsString(pass_item) && pass_item->valuestring != NULL)
    {
        strlcpy(
            (char *)app->wifi_cfg_buf.sta.password,
            pass_item->valuestring,
            sizeof(app->wifi_cfg_buf.sta.password));
        app->wifi_cfg_buf.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    }
    else
    {
        app->wifi_cfg_buf.sta.threshold.authmode = WIFI_AUTH_OPEN;
    }
    cJSON_Delete(root);

    esp_wifi_disconnect();
    vTaskDelay(pdMS_TO_TICKS(2000));

    esp_err_t err = esp_wifi_set_config(WIFI_IF_STA, &app->wifi_cfg_buf);
    if (err != ESP_OK)
    {
        hyLogError(
            WIFIMAN_HTTP_TAG,
            "%s: 500, set config failed",
            _hy_wifiman_post_api_connect.uri);
        return _hyWifimanSendErrorResponse(req, 500, "Failed to set config");
    }

    err = esp_wifi_connect();
    if (err != ESP_OK)
    {
        hyLogError(
            WIFIMAN_HTTP_TAG,
            "%s: 500, connect start failed",
            _hy_wifiman_post_api_connect.uri);
        return _hyWifimanSendErrorResponse(req, 500, "Failed to start connection");
    }
    app->reason = 0;
    app->connecting = true;

    hyLogInfo(
        WIFIMAN_HTTP_TAG,
        "%s: 200, connecting to %s...",
        _hy_wifiman_post_api_connect.uri,
        (char *)app->wifi_cfg_buf.sta.ssid);
    return httpd_resp_send(req, "", 0);
}

esp_err_t _hyWifimanPostAPICommit(httpd_req_t *req)
{
    HyWifiman *app = (HyWifiman *)req->user_ctx;
    esp_err_t err;

    if (app->connecting)
    {
        hyLogWarn(
            WIFIMAN_HTTP_TAG,
            "%s: 409, connecting in progress",
            _hy_wifiman_post_api_commit.uri);
        return _hyWifimanSendErrorResponse(req, 409, "Connecting in progress");
    }

    if (strlen((char *)app->wifi_cfg_buf.sta.ssid) == 0)
    {
        hyLogWarn(
            WIFIMAN_HTTP_TAG,
            "%s: 400, commit failed: no SSID in buffer",
            _hy_wifiman_post_api_commit.uri);
        return _hyWifimanSendErrorResponse(req, 400, "No configuration to commit");
    }

    err = nvs_set_str(app->nvs, "wm_sta_ssid", (char *)app->wifi_cfg_buf.sta.ssid);
    if (err == ESP_OK)
        err = nvs_set_str(app->nvs, "wm_sta_pass", (char *)app->wifi_cfg_buf.sta.password);
    if (err == ESP_OK)
        err = nvs_commit(app->nvs);
    if (err != ESP_OK)
    {
        hyLogError(
            WIFIMAN_HTTP_TAG,
            "%s: 500, NVS write failed: %s",
            _hy_wifiman_post_api_commit.uri,
            esp_err_to_name(err));
        return _hyWifimanSendErrorResponse(req, 500, "Failed to save credentials");
    }
    hyLogInfo(
        WIFIMAN_HTTP_TAG,
        "%s: credentials committed to NVS, switching to STA mode...",
        _hy_wifiman_post_api_commit.uri);

    err = httpd_resp_send(req, "", 0);
    if (err != ESP_OK)
        return err;
    vTaskDelay(pdMS_TO_TICKS(500));

    esp_wifi_disconnect();
    vTaskDelay(pdMS_TO_TICKS(2000));

    esp_wifi_set_config(WIFI_IF_STA, &app->wifi_cfg_buf);
    esp_wifi_set_mode(WIFI_MODE_STA);
    err = esp_wifi_connect();
    if (err != ESP_OK)
    {
        hyLogError(
            WIFIMAN_HTTP_TAG,
            "%s: 500, connect start failed: %s",
            _hy_wifiman_post_api_connect.uri,
            esp_err_to_name(err));
        return err;
    }
    app->reason = 0;
    app->connecting = true;

    return ESP_OK;
}

esp_err_t _hyWifimanSendErrorResponse(httpd_req_t *req, uint16_t code, const char *msg)
{
    httpd_resp_set_type(req, "application/json");
    switch (code)
    {
    case 400:
        httpd_resp_set_status(req, "400 Bad Request");
        break;
    case 409:
        httpd_resp_set_status(req, "409 Conflict");
        break;
    case 500:
        httpd_resp_set_status(req, "500 Internal Server Error");
        break;
    default:
        httpd_resp_set_status(req, "500 Internal Server Error");
        break;
    }

    char json_buf[256];
    int len = snprintf(
        json_buf, sizeof(json_buf),
        "{\"error\":\"%s\"}", msg);

    if (len >= sizeof(json_buf))
        len = sizeof(json_buf) - 1;

    return httpd_resp_send(req, json_buf, len);
}
