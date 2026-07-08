#include "infrastructure/device/wifi/esp_wifi_impl.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "domain/contracts/device/wifi.h"
#include "domain/models/error.h"
#include "domain/models/wifi.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_wifi.h"

#define ESP_WIFI_IMPL_ACTOR "esp_wifi"

/* Helper Function Prototypes */

static dom_models_error_t*              bad_argument_error(const char* msg);
static dom_models_error_t*              error_from_esp(const char* action, esp_err_t err);
static size_t                           bounded_strlen(const char* value, size_t max_len);
static size_t                           bounded_byte_strlen(const uint8_t* value, size_t max_len);
static void                             copy_cstr(char* dst, size_t dst_size, const char* src);
static void                             copy_cstr_to_bytes(uint8_t* dst, size_t dst_size, const char* src);
static void                             copy_bytes_to_cstr(char* dst, size_t dst_size, const uint8_t* src, size_t src_size);
static bool                             wifi_mode_from_domain(dom_models_wifi_mode_t in, wifi_mode_t* out);
static dom_models_wifi_mode_t           wifi_mode_to_domain(wifi_mode_t in);
static bool                             wifi_auth_from_domain(dom_models_wifi_auth_mode_t in, wifi_auth_mode_t* out);
static dom_models_wifi_auth_mode_t      wifi_auth_to_domain(wifi_auth_mode_t in);
static dom_models_wifi_cipher_t         wifi_cipher_to_domain(wifi_cipher_type_t in);
static dom_models_wifi_bandwidth_t      wifi_bandwidth_to_domain(wifi_bandwidth_t in);
static dom_models_wifi_second_channel_t wifi_second_channel_to_domain(wifi_second_chan_t in);
static uint32_t                         phy_flags_from_ap_record(const wifi_ap_record_t* record);
static uint32_t                         phy_flags_from_sta_info(const wifi_sta_info_t* sta);
static void                             copy_if_key(bool* available, char* out, size_t out_size, const char* if_key);
static void                             copy_ap_record(dom_models_wifi_ap_record_t* out, const wifi_ap_record_t* in);
static void                             copy_ap_client(dom_models_wifi_ap_client_t* out, const wifi_sta_info_t* in);
static esp_err_t                        ensure_sta_mode(void);
static esp_err_t                        ensure_ap_mode(void);
static void                             clear_scan_records(dom_models_wifi_scan_result_t* out);
static void                             load_scan_records(inf_device_wifi_esp_wifi_impl_ctx_t* ctx);

/* Event Handler Function Prototypes */

static void wifi_event_handler(void* arg, esp_event_base_t base, int32_t id, void* data);
static void on_scan_done(inf_device_wifi_esp_wifi_impl_ctx_t* ctx, const wifi_event_sta_scan_done_t* event);
static void on_sta_connected(inf_device_wifi_esp_wifi_impl_ctx_t* ctx);
static void on_sta_disconnected(inf_device_wifi_esp_wifi_impl_ctx_t* ctx);
static void on_ap_start(inf_device_wifi_esp_wifi_impl_ctx_t* ctx);
static void on_ap_stop(inf_device_wifi_esp_wifi_impl_ctx_t* ctx);

/* Contract Function Prototypes */

static dom_models_error_t* start_impl(
    dom_contracts_device_wifi_t* self
);
static dom_models_error_t* stop_impl(
    dom_contracts_device_wifi_t* self
);
static dom_models_error_t* set_mode_impl(
    dom_contracts_device_wifi_t* self,
    dom_models_wifi_mode_t       mode
);
static dom_models_error_t* get_status_impl(
    dom_contracts_device_wifi_t* self,
    dom_models_wifi_status_t*    out
);
static dom_models_error_t* connect_sta_impl(
    dom_contracts_device_wifi_t*                self,
    const dom_models_wifi_sta_connect_config_t* config
);
static dom_models_error_t* disconnect_sta_impl(
    dom_contracts_device_wifi_t* self
);
static dom_models_error_t* start_ap_impl(
    dom_contracts_device_wifi_t*       self,
    const dom_models_wifi_ap_config_t* config
);
static dom_models_error_t* stop_ap_impl(
    dom_contracts_device_wifi_t* self
);
static dom_models_error_t* start_scan_impl(
    dom_contracts_device_wifi_t*         self,
    const dom_models_wifi_scan_config_t* config
);
static dom_models_error_t* get_scanned_impl(
    dom_contracts_device_wifi_t*   self,
    dom_models_wifi_scan_result_t* out
);

/* Constructor and Destructor */

dom_contracts_device_wifi_t* inf_device_wifi_esp_wifi_impl_new(const inf_device_wifi_esp_wifi_impl_cfg_t* cfg) {
    inf_device_wifi_esp_wifi_impl_ctx_t* ctx = (inf_device_wifi_esp_wifi_impl_ctx_t*)calloc(1, sizeof(inf_device_wifi_esp_wifi_impl_ctx_t));
    if (!ctx) {
        return NULL;
    }

    if (!cfg) {
        inf_device_wifi_esp_wifi_impl_cfg_t default_cfg = INF_DEVICE_WIFI_ESP_WIFI_IMPL_CFG_DEFAULT();
        memcpy(&ctx->cfg, &default_cfg, sizeof(inf_device_wifi_esp_wifi_impl_cfg_t));
    } else {
        memcpy(&ctx->cfg, cfg, sizeof(inf_device_wifi_esp_wifi_impl_cfg_t));
    }

    ctx->scanned.status = DOM_MODELS_WIFI_SCAN_STATUS_IDLE;

    if (ctx->cfg.register_event_handler) {
        esp_err_t err = esp_event_handler_instance_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            wifi_event_handler,
            ctx,
            &ctx->wifi_event_handler
        );
        if (err != ESP_OK) {
            free(ctx);
            return NULL;
        }
        ctx->wifi_event_handler_registered = true;
    }

    dom_contracts_device_wifi_t* self = dom_contracts_device_wifi_new(ctx);
    if (!self) {
        if (ctx->wifi_event_handler_registered) {
            esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, ctx->wifi_event_handler);
        }
        free(ctx);
        return NULL;
    }

    self->start          = start_impl;
    self->stop           = stop_impl;
    self->set_mode       = set_mode_impl;
    self->get_status     = get_status_impl;
    self->connect_sta    = connect_sta_impl;
    self->disconnect_sta = disconnect_sta_impl;
    self->start_ap       = start_ap_impl;
    self->stop_ap        = stop_ap_impl;
    self->start_scan     = start_scan_impl;
    self->get_scanned    = get_scanned_impl;

    return self;
}

void inf_device_wifi_esp_wifi_impl_delete(dom_contracts_device_wifi_t* self) {
    if (!self || !self->ctx) {
        return;
    }

    inf_device_wifi_esp_wifi_impl_ctx_t* ctx = self->ctx;

    if (ctx->wifi_event_handler_registered) {
        esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, ctx->wifi_event_handler);
    }

    free(ctx);
    dom_contracts_device_wifi_delete(self);
}

/* Contract Function Implementations */

static dom_models_error_t* start_impl(
    dom_contracts_device_wifi_t* self
) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing wifi context");
    }

    inf_device_wifi_esp_wifi_impl_ctx_t* ctx = self->ctx;

    esp_err_t err = esp_wifi_start();
    if (err != ESP_OK) {
        return error_from_esp("esp_wifi_start", err);
    }

    ctx->started = true;

    return NULL;
}

static dom_models_error_t* stop_impl(
    dom_contracts_device_wifi_t* self
) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing wifi context");
    }

    inf_device_wifi_esp_wifi_impl_ctx_t* ctx = self->ctx;

    esp_err_t err = esp_wifi_stop();
    if (err != ESP_OK) {
        return error_from_esp("esp_wifi_stop", err);
    }

    ctx->started       = false;
    ctx->sta_connected = false;
    ctx->ap_started    = false;
    memset(&ctx->scanned, 0, sizeof(dom_models_wifi_scan_result_t));
    ctx->scanned.status = DOM_MODELS_WIFI_SCAN_STATUS_IDLE;

    return NULL;
}

static dom_models_error_t* set_mode_impl(
    dom_contracts_device_wifi_t* self,
    dom_models_wifi_mode_t       mode
) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing wifi context");
    }

    wifi_mode_t wifi_mode;
    if (!wifi_mode_from_domain(mode, &wifi_mode)) {
        return bad_argument_error("invalid wifi mode");
    }

    esp_err_t err = esp_wifi_set_mode(wifi_mode);
    if (err != ESP_OK) {
        return error_from_esp("esp_wifi_set_mode", err);
    }

    return NULL;
}

static dom_models_error_t* get_status_impl(
    dom_contracts_device_wifi_t* self,
    dom_models_wifi_status_t*    out
) {
    if (!self || !self->ctx || !out) {
        return bad_argument_error("missing wifi status argument");
    }

    inf_device_wifi_esp_wifi_impl_ctx_t* ctx = self->ctx;

    memset(out, 0, sizeof(dom_models_wifi_status_t));

    wifi_mode_t mode;
    esp_err_t   err = esp_wifi_get_mode(&mode);
    if (err != ESP_OK) {
        return error_from_esp("esp_wifi_get_mode", err);
    }

    out->mode      = wifi_mode_to_domain(mode);
    out->started   = ctx->started;
    out->connected = ctx->sta_connected;

    copy_if_key(&out->sta_if_key_available, out->sta_if_key, sizeof(out->sta_if_key), ctx->cfg.sta_if_key);
    copy_if_key(&out->ap_if_key_available, out->ap_if_key, sizeof(out->ap_if_key), ctx->cfg.ap_if_key);

    err = esp_wifi_get_mac(WIFI_IF_STA, out->sta_mac);
    if (err == ESP_OK) {
        out->sta_mac_available = true;
    }

    err = esp_wifi_get_mac(WIFI_IF_AP, out->ap_mac);
    if (err == ESP_OK) {
        out->ap_mac_available = true;
    }

    wifi_ap_record_t ap_record;
    memset(&ap_record, 0, sizeof(wifi_ap_record_t));
    err = esp_wifi_sta_get_ap_info(&ap_record);
    if (err == ESP_OK) {
        out->connected              = true;
        out->connected_ap_available = true;
        copy_ap_record(&out->connected_ap, &ap_record);
    }

    wifi_sta_list_t sta_list;
    memset(&sta_list, 0, sizeof(wifi_sta_list_t));
    err = esp_wifi_ap_get_sta_list(&sta_list);
    if (err == ESP_OK) {
        out->ap_client_total_count = (size_t)sta_list.num;
        out->ap_client_count       = (size_t)sta_list.num;
        if (out->ap_client_count > DOM_MODELS_WIFI_AP_CLIENT_MAX) {
            out->ap_client_count      = DOM_MODELS_WIFI_AP_CLIENT_MAX;
            out->ap_clients_truncated = true;
        }

        for (size_t i = 0; i < out->ap_client_count; i++) {
            copy_ap_client(&out->ap_clients[i], &sta_list.sta[i]);
        }
    }

    return NULL;
}

static dom_models_error_t* connect_sta_impl(
    dom_contracts_device_wifi_t*                self,
    const dom_models_wifi_sta_connect_config_t* config
) {
    if (!self || !self->ctx || !config) {
        return bad_argument_error("missing wifi sta config");
    }

    if (bounded_strlen(config->ssid, sizeof(config->ssid)) == 0) {
        return bad_argument_error("missing wifi sta ssid");
    }

    esp_err_t err = ensure_sta_mode();
    if (err != ESP_OK) {
        return error_from_esp("esp_wifi_set_mode", err);
    }

    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config_t));

    copy_cstr_to_bytes(wifi_config.sta.ssid, sizeof(wifi_config.sta.ssid), config->ssid);
    copy_cstr_to_bytes(wifi_config.sta.password, sizeof(wifi_config.sta.password), config->password);
    wifi_config.sta.bssid_set = config->bssid_set;
    if (config->bssid_set) {
        memcpy(wifi_config.sta.bssid, config->bssid, sizeof(wifi_config.sta.bssid));
    }
    if (config->channel_set) {
        wifi_config.sta.channel = config->channel;
    }

    err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (err != ESP_OK) {
        return error_from_esp("esp_wifi_set_config", err);
    }

    err = esp_wifi_connect();
    if (err != ESP_OK) {
        return error_from_esp("esp_wifi_connect", err);
    }

    return NULL;
}

static dom_models_error_t* disconnect_sta_impl(
    dom_contracts_device_wifi_t* self
) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing wifi context");
    }

    esp_err_t err = esp_wifi_disconnect();
    if (err != ESP_OK) {
        return error_from_esp("esp_wifi_disconnect", err);
    }

    inf_device_wifi_esp_wifi_impl_ctx_t* ctx = self->ctx;
    ctx->sta_connected                       = false;

    return NULL;
}

static dom_models_error_t* start_ap_impl(
    dom_contracts_device_wifi_t*       self,
    const dom_models_wifi_ap_config_t* config
) {
    if (!self || !self->ctx || !config) {
        return bad_argument_error("missing wifi ap config");
    }

    if (bounded_strlen(config->ssid, sizeof(config->ssid)) == 0) {
        return bad_argument_error("missing wifi ap ssid");
    }

    wifi_auth_mode_t auth_mode;
    if (!wifi_auth_from_domain(config->auth_mode, &auth_mode)) {
        return bad_argument_error("invalid wifi ap auth mode");
    }

    esp_err_t err = ensure_ap_mode();
    if (err != ESP_OK) {
        return error_from_esp("esp_wifi_set_mode", err);
    }

    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config_t));

    copy_cstr_to_bytes(wifi_config.ap.ssid, sizeof(wifi_config.ap.ssid), config->ssid);
    copy_cstr_to_bytes(wifi_config.ap.password, sizeof(wifi_config.ap.password), config->password);
    wifi_config.ap.ssid_len       = (uint8_t)bounded_strlen(config->ssid, sizeof(wifi_config.ap.ssid));
    wifi_config.ap.channel        = config->channel;
    wifi_config.ap.authmode       = auth_mode;
    wifi_config.ap.max_connection = config->max_clients;
    wifi_config.ap.ssid_hidden    = config->hidden ? 1 : 0;

    err = esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    if (err != ESP_OK) {
        return error_from_esp("esp_wifi_set_config", err);
    }

    inf_device_wifi_esp_wifi_impl_ctx_t* ctx = self->ctx;
    if (!ctx->started) {
        err = esp_wifi_start();
        if (err != ESP_OK) {
            return error_from_esp("esp_wifi_start", err);
        }
        ctx->started = true;
    }

    return NULL;
}

static dom_models_error_t* stop_ap_impl(
    dom_contracts_device_wifi_t* self
) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing wifi context");
    }

    inf_device_wifi_esp_wifi_impl_ctx_t* ctx = self->ctx;

    wifi_mode_t mode;
    esp_err_t   err = esp_wifi_get_mode(&mode);
    if (err != ESP_OK) {
        return error_from_esp("esp_wifi_get_mode", err);
    }

    if (mode == WIFI_MODE_APSTA) {
        err = esp_wifi_set_mode(WIFI_MODE_STA);
        if (err != ESP_OK) {
            return error_from_esp("esp_wifi_set_mode", err);
        }
        ctx->ap_started = false;
        return NULL;
    }

    if (mode == WIFI_MODE_AP) {
        err = esp_wifi_stop();
        if (err != ESP_OK) {
            return error_from_esp("esp_wifi_stop", err);
        }
        ctx->started    = false;
        ctx->ap_started = false;
    }

    return NULL;
}

static dom_models_error_t* start_scan_impl(
    dom_contracts_device_wifi_t*         self,
    const dom_models_wifi_scan_config_t* config
) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing wifi context");
    }

    inf_device_wifi_esp_wifi_impl_ctx_t* ctx = self->ctx;

    memset(&ctx->scanned, 0, sizeof(dom_models_wifi_scan_result_t));
    ctx->scanned.status = DOM_MODELS_WIFI_SCAN_STATUS_RUNNING;

    wifi_scan_config_t scan_config;
    memset(&scan_config, 0, sizeof(wifi_scan_config_t));

    uint8_t ssid[DOM_MODELS_WIFI_SSID_LEN];
    uint8_t bssid[DOM_MODELS_WIFI_MAC_LEN];
    memset(ssid, 0, sizeof(ssid));
    memset(bssid, 0, sizeof(bssid));

    if (config) {
        if (config->ssid_set) {
            copy_cstr_to_bytes(ssid, sizeof(ssid), config->ssid);
            scan_config.ssid = ssid;
        }
        if (config->bssid_set) {
            memcpy(bssid, config->bssid, sizeof(bssid));
            scan_config.bssid = bssid;
        }
        if (config->channel_set) {
            scan_config.channel = config->channel;
        }
        scan_config.scan_type = config->passive ? WIFI_SCAN_TYPE_PASSIVE : WIFI_SCAN_TYPE_ACTIVE;
        if (config->timeout_ms > 0) {
            if (config->passive) {
                scan_config.scan_time.passive = config->timeout_ms;
            } else {
                scan_config.scan_time.active.max = config->timeout_ms;
            }
        }
    }

    esp_err_t err = esp_wifi_scan_start(&scan_config, false);
    if (err != ESP_OK) {
        ctx->scanned.status        = DOM_MODELS_WIFI_SCAN_STATUS_FAILED;
        ctx->scanned.driver_status = (uint32_t)err;
        return error_from_esp("esp_wifi_scan_start", err);
    }

    return NULL;
}

static dom_models_error_t* get_scanned_impl(
    dom_contracts_device_wifi_t*   self,
    dom_models_wifi_scan_result_t* out
) {
    if (!self || !self->ctx || !out) {
        return bad_argument_error("missing wifi scan result argument");
    }

    inf_device_wifi_esp_wifi_impl_ctx_t* ctx = self->ctx;
    memcpy(out, &ctx->scanned, sizeof(dom_models_wifi_scan_result_t));

    return NULL;
}

/* Event Handler Function Implementations */

static void wifi_event_handler(void* arg, esp_event_base_t base, int32_t id, void* data) {
    if (!arg || base != WIFI_EVENT) {
        return;
    }

    inf_device_wifi_esp_wifi_impl_ctx_t* ctx = arg;

    switch (id) {
        case WIFI_EVENT_SCAN_DONE:
            on_scan_done(ctx, (const wifi_event_sta_scan_done_t*)data);
            break;
        case WIFI_EVENT_STA_CONNECTED:
            on_sta_connected(ctx);
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            on_sta_disconnected(ctx);
            break;
        case WIFI_EVENT_AP_START:
            on_ap_start(ctx);
            break;
        case WIFI_EVENT_AP_STOP:
            on_ap_stop(ctx);
            break;
        default:
            break;
    }
}

static void on_scan_done(inf_device_wifi_esp_wifi_impl_ctx_t* ctx, const wifi_event_sta_scan_done_t* event) {
    if (!ctx) {
        return;
    }

    clear_scan_records(&ctx->scanned);

    if (!event) {
        ctx->scanned.status        = DOM_MODELS_WIFI_SCAN_STATUS_FAILED;
        ctx->scanned.driver_status = 1;
        return;
    }

    ctx->scanned.scan_id       = event->scan_id;
    ctx->scanned.driver_status = event->status;

    if (event->status != 0) {
        ctx->scanned.status = DOM_MODELS_WIFI_SCAN_STATUS_FAILED;
        return;
    }

    ctx->scanned.status      = DOM_MODELS_WIFI_SCAN_STATUS_DONE;
    ctx->scanned.total_count = event->number;
    load_scan_records(ctx);
}

static void on_sta_connected(inf_device_wifi_esp_wifi_impl_ctx_t* ctx) {
    if (!ctx) {
        return;
    }

    ctx->sta_connected = true;
}

static void on_sta_disconnected(inf_device_wifi_esp_wifi_impl_ctx_t* ctx) {
    if (!ctx) {
        return;
    }

    ctx->sta_connected = false;
}

static void on_ap_start(inf_device_wifi_esp_wifi_impl_ctx_t* ctx) {
    if (!ctx) {
        return;
    }

    ctx->ap_started = true;
    ctx->started    = true;
}

static void on_ap_stop(inf_device_wifi_esp_wifi_impl_ctx_t* ctx) {
    if (!ctx) {
        return;
    }

    ctx->ap_started = false;
}

/* Helper Function Implementations */

static dom_models_error_t* bad_argument_error(const char* msg) {
    return dom_models_error_new(ESP_WIFI_IMPL_ACTOR, DOMAIN_MODELS_ERROR_TYPE_BAD_ARGUMENT, "%s", msg ? msg : "bad argument");
}

static dom_models_error_t* error_from_esp(const char* action, esp_err_t err) {
    if (err == ESP_OK) {
        return NULL;
    }

    return dom_models_error_new(ESP_WIFI_IMPL_ACTOR, DOMAIN_MODELS_ERROR_TYPE_SYSTEM_FAILURE, "%s failed: esp_err=0x%x", action ? action : "esp_wifi", (unsigned int)err);
}

static size_t bounded_strlen(const char* value, size_t max_len) {
    size_t len = 0;

    if (!value) {
        return 0;
    }

    while (len < max_len && value[len] != '\0') {
        len++;
    }

    return len;
}

static size_t bounded_byte_strlen(const uint8_t* value, size_t max_len) {
    size_t len = 0;

    if (!value) {
        return 0;
    }

    while (len < max_len && value[len] != '\0') {
        len++;
    }

    return len;
}

static void copy_cstr(char* dst, size_t dst_size, const char* src) {
    if (!dst || dst_size == 0) {
        return;
    }

    const char* value = src ? src : "";
    size_t      len   = bounded_strlen(value, dst_size - 1);
    dst[len]          = '\0';

    if (len > 0) {
        memcpy(dst, value, len);
    }
}

static void copy_cstr_to_bytes(uint8_t* dst, size_t dst_size, const char* src) {
    if (!dst || dst_size == 0 || !src) {
        return;
    }

    size_t len = bounded_strlen(src, dst_size);
    if (len > 0) {
        memcpy(dst, src, len);
    }
}

static void copy_bytes_to_cstr(char* dst, size_t dst_size, const uint8_t* src, size_t src_size) {
    if (!dst || dst_size == 0) {
        return;
    }

    size_t len      = bounded_byte_strlen(src, src_size);
    size_t copy_len = len < dst_size - 1 ? len : dst_size - 1;
    dst[copy_len]   = '\0';

    if (copy_len > 0) {
        memcpy(dst, src, copy_len);
    }
}

static bool wifi_mode_from_domain(dom_models_wifi_mode_t in, wifi_mode_t* out) {
    if (!out) {
        return false;
    }

    switch (in) {
        case DOM_MODELS_WIFI_MODE_NULL:
            *out = WIFI_MODE_NULL;
            return true;
        case DOM_MODELS_WIFI_MODE_STA:
            *out = WIFI_MODE_STA;
            return true;
        case DOM_MODELS_WIFI_MODE_AP:
            *out = WIFI_MODE_AP;
            return true;
        case DOM_MODELS_WIFI_MODE_APSTA:
            *out = WIFI_MODE_APSTA;
            return true;
        case DOM_MODELS_WIFI_MODE_NAN:
            *out = WIFI_MODE_NAN;
            return true;
    }

    return false;
}

static dom_models_wifi_mode_t wifi_mode_to_domain(wifi_mode_t in) {
    switch (in) {
        case WIFI_MODE_NULL:
            return DOM_MODELS_WIFI_MODE_NULL;
        case WIFI_MODE_STA:
            return DOM_MODELS_WIFI_MODE_STA;
        case WIFI_MODE_AP:
            return DOM_MODELS_WIFI_MODE_AP;
        case WIFI_MODE_APSTA:
            return DOM_MODELS_WIFI_MODE_APSTA;
        case WIFI_MODE_NAN:
            return DOM_MODELS_WIFI_MODE_NAN;
        default:
            return DOM_MODELS_WIFI_MODE_NULL;
    }
}

static bool wifi_auth_from_domain(dom_models_wifi_auth_mode_t in, wifi_auth_mode_t* out) {
    if (!out) {
        return false;
    }

    switch (in) {
        case DOM_MODELS_WIFI_AUTH_OPEN:
            *out = WIFI_AUTH_OPEN;
            return true;
        case DOM_MODELS_WIFI_AUTH_WEP:
            *out = WIFI_AUTH_WEP;
            return true;
        case DOM_MODELS_WIFI_AUTH_WPA_PSK:
            *out = WIFI_AUTH_WPA_PSK;
            return true;
        case DOM_MODELS_WIFI_AUTH_WPA2_PSK:
            *out = WIFI_AUTH_WPA2_PSK;
            return true;
        case DOM_MODELS_WIFI_AUTH_WPA_WPA2_PSK:
            *out = WIFI_AUTH_WPA_WPA2_PSK;
            return true;
        case DOM_MODELS_WIFI_AUTH_WPA_ENTERPRISE:
            *out = WIFI_AUTH_WPA_ENTERPRISE;
            return true;
        case DOM_MODELS_WIFI_AUTH_WPA2_ENTERPRISE:
            *out = WIFI_AUTH_WPA2_ENTERPRISE;
            return true;
        case DOM_MODELS_WIFI_AUTH_WPA3_PSK:
            *out = WIFI_AUTH_WPA3_PSK;
            return true;
        case DOM_MODELS_WIFI_AUTH_WPA2_WPA3_PSK:
            *out = WIFI_AUTH_WPA2_WPA3_PSK;
            return true;
        case DOM_MODELS_WIFI_AUTH_WAPI_PSK:
            *out = WIFI_AUTH_WAPI_PSK;
            return true;
        case DOM_MODELS_WIFI_AUTH_OWE:
            *out = WIFI_AUTH_OWE;
            return true;
        case DOM_MODELS_WIFI_AUTH_WPA3_ENT_192:
            *out = WIFI_AUTH_WPA3_ENT_192;
            return true;
        case DOM_MODELS_WIFI_AUTH_DPP:
            *out = WIFI_AUTH_DPP;
            return true;
        case DOM_MODELS_WIFI_AUTH_WPA3_ENTERPRISE:
            *out = WIFI_AUTH_WPA3_ENTERPRISE;
            return true;
        case DOM_MODELS_WIFI_AUTH_WPA2_WPA3_ENTERPRISE:
            *out = WIFI_AUTH_WPA2_WPA3_ENTERPRISE;
            return true;
        case DOM_MODELS_WIFI_AUTH_UNKNOWN:
            return false;
    }

    return false;
}

static dom_models_wifi_auth_mode_t wifi_auth_to_domain(wifi_auth_mode_t in) {
    switch (in) {
        case WIFI_AUTH_OPEN:
            return DOM_MODELS_WIFI_AUTH_OPEN;
        case WIFI_AUTH_WEP:
            return DOM_MODELS_WIFI_AUTH_WEP;
        case WIFI_AUTH_WPA_PSK:
            return DOM_MODELS_WIFI_AUTH_WPA_PSK;
        case WIFI_AUTH_WPA2_PSK:
            return DOM_MODELS_WIFI_AUTH_WPA2_PSK;
        case WIFI_AUTH_WPA_WPA2_PSK:
            return DOM_MODELS_WIFI_AUTH_WPA_WPA2_PSK;
        case WIFI_AUTH_WPA_ENTERPRISE:
            return DOM_MODELS_WIFI_AUTH_WPA_ENTERPRISE;
        case WIFI_AUTH_WPA3_PSK:
            return DOM_MODELS_WIFI_AUTH_WPA3_PSK;
        case WIFI_AUTH_WPA2_WPA3_PSK:
            return DOM_MODELS_WIFI_AUTH_WPA2_WPA3_PSK;
        case WIFI_AUTH_WAPI_PSK:
            return DOM_MODELS_WIFI_AUTH_WAPI_PSK;
        case WIFI_AUTH_OWE:
            return DOM_MODELS_WIFI_AUTH_OWE;
        case WIFI_AUTH_WPA3_ENT_192:
            return DOM_MODELS_WIFI_AUTH_WPA3_ENT_192;
        case WIFI_AUTH_DPP:
            return DOM_MODELS_WIFI_AUTH_DPP;
        case WIFI_AUTH_WPA3_ENTERPRISE:
            return DOM_MODELS_WIFI_AUTH_WPA3_ENTERPRISE;
        case WIFI_AUTH_WPA2_WPA3_ENTERPRISE:
            return DOM_MODELS_WIFI_AUTH_WPA2_WPA3_ENTERPRISE;
        default:
            return DOM_MODELS_WIFI_AUTH_UNKNOWN;
    }
}

static dom_models_wifi_cipher_t wifi_cipher_to_domain(wifi_cipher_type_t in) {
    switch (in) {
        case WIFI_CIPHER_TYPE_NONE:
            return DOM_MODELS_WIFI_CIPHER_NONE;
        case WIFI_CIPHER_TYPE_WEP40:
            return DOM_MODELS_WIFI_CIPHER_WEP40;
        case WIFI_CIPHER_TYPE_WEP104:
            return DOM_MODELS_WIFI_CIPHER_WEP104;
        case WIFI_CIPHER_TYPE_TKIP:
            return DOM_MODELS_WIFI_CIPHER_TKIP;
        case WIFI_CIPHER_TYPE_CCMP:
            return DOM_MODELS_WIFI_CIPHER_CCMP;
        case WIFI_CIPHER_TYPE_TKIP_CCMP:
            return DOM_MODELS_WIFI_CIPHER_TKIP_CCMP;
        case WIFI_CIPHER_TYPE_AES_CMAC128:
            return DOM_MODELS_WIFI_CIPHER_AES_CMAC128;
        case WIFI_CIPHER_TYPE_SMS4:
            return DOM_MODELS_WIFI_CIPHER_SMS4;
        case WIFI_CIPHER_TYPE_GCMP:
            return DOM_MODELS_WIFI_CIPHER_GCMP;
        case WIFI_CIPHER_TYPE_GCMP256:
            return DOM_MODELS_WIFI_CIPHER_GCMP256;
        case WIFI_CIPHER_TYPE_AES_GMAC128:
            return DOM_MODELS_WIFI_CIPHER_AES_GMAC128;
        case WIFI_CIPHER_TYPE_AES_GMAC256:
            return DOM_MODELS_WIFI_CIPHER_AES_GMAC256;
        default:
            return DOM_MODELS_WIFI_CIPHER_UNKNOWN;
    }
}

static dom_models_wifi_bandwidth_t wifi_bandwidth_to_domain(wifi_bandwidth_t in) {
    switch (in) {
        case WIFI_BW20:
            return DOM_MODELS_WIFI_BANDWIDTH_20MHZ;
        case WIFI_BW40:
            return DOM_MODELS_WIFI_BANDWIDTH_40MHZ;
        case WIFI_BW80:
            return DOM_MODELS_WIFI_BANDWIDTH_80MHZ;
        case WIFI_BW160:
            return DOM_MODELS_WIFI_BANDWIDTH_160MHZ;
        case WIFI_BW80_BW80:
            return DOM_MODELS_WIFI_BANDWIDTH_80_80MHZ;
        default:
            return DOM_MODELS_WIFI_BANDWIDTH_UNKNOWN;
    }
}

static dom_models_wifi_second_channel_t wifi_second_channel_to_domain(wifi_second_chan_t in) {
    switch (in) {
        case WIFI_SECOND_CHAN_ABOVE:
            return DOM_MODELS_WIFI_SECOND_CHANNEL_ABOVE;
        case WIFI_SECOND_CHAN_BELOW:
            return DOM_MODELS_WIFI_SECOND_CHANNEL_BELOW;
        case WIFI_SECOND_CHAN_NONE:
        default:
            return DOM_MODELS_WIFI_SECOND_CHANNEL_NONE;
    }
}

static uint32_t phy_flags_from_ap_record(const wifi_ap_record_t* record) {
    uint32_t flags = 0;

    if (!record) {
        return flags;
    }

    if (record->phy_11b) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_11B;
    }
    if (record->phy_11g) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_11G;
    }
    if (record->phy_11n) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_11N;
    }
    if (record->phy_lr) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_LR;
    }
    if (record->phy_11a) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_11A;
    }
    if (record->phy_11ac) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_11AC;
    }
    if (record->phy_11ax) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_11AX;
    }
    if (record->wps) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_WPS;
    }
    if (record->ftm_responder) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_FTM_RESPONDER;
    }
    if (record->ftm_initiator) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_FTM_INITIATOR;
    }

    return flags;
}

static uint32_t phy_flags_from_sta_info(const wifi_sta_info_t* sta) {
    uint32_t flags = 0;

    if (!sta) {
        return flags;
    }

    if (sta->phy_11b) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_11B;
    }
    if (sta->phy_11g) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_11G;
    }
    if (sta->phy_11n) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_11N;
    }
    if (sta->phy_lr) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_LR;
    }
    if (sta->phy_11a) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_11A;
    }
    if (sta->phy_11ac) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_11AC;
    }
    if (sta->phy_11ax) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_11AX;
    }

    return flags;
}

static void copy_if_key(bool* available, char* out, size_t out_size, const char* if_key) {
    if (!available || !out || out_size == 0) {
        return;
    }

    if (!if_key || if_key[0] == '\0') {
        *available = false;
        out[0]     = '\0';
        return;
    }

    *available = true;
    copy_cstr(out, out_size, if_key);
}

static void copy_ap_record(dom_models_wifi_ap_record_t* out, const wifi_ap_record_t* in) {
    if (!out || !in) {
        return;
    }

    memset(out, 0, sizeof(dom_models_wifi_ap_record_t));

    out->bssid_available = true;
    memcpy(out->bssid, in->bssid, sizeof(out->bssid));
    copy_bytes_to_cstr(out->ssid, sizeof(out->ssid), in->ssid, sizeof(in->ssid));
    out->primary_channel = in->primary;
    out->second_channel  = wifi_second_channel_to_domain(in->second);
    out->rssi            = in->rssi;
    out->auth_mode       = wifi_auth_to_domain(in->authmode);
    out->pairwise_cipher = wifi_cipher_to_domain(in->pairwise_cipher);
    out->group_cipher    = wifi_cipher_to_domain(in->group_cipher);
    out->bandwidth       = wifi_bandwidth_to_domain(in->bandwidth);
    out->phy_flags       = phy_flags_from_ap_record(in);
}

static void copy_ap_client(dom_models_wifi_ap_client_t* out, const wifi_sta_info_t* in) {
    if (!out || !in) {
        return;
    }

    memset(out, 0, sizeof(dom_models_wifi_ap_client_t));

    memcpy(out->mac, in->mac, sizeof(out->mac));
    out->rssi      = in->rssi;
    out->phy_flags = phy_flags_from_sta_info(in);
}

static esp_err_t ensure_sta_mode(void) {
    wifi_mode_t mode;
    esp_err_t   err = esp_wifi_get_mode(&mode);
    if (err != ESP_OK) {
        return err;
    }

    if (mode == WIFI_MODE_STA || mode == WIFI_MODE_APSTA) {
        return ESP_OK;
    }

    if (mode == WIFI_MODE_AP) {
        return esp_wifi_set_mode(WIFI_MODE_APSTA);
    }

    return esp_wifi_set_mode(WIFI_MODE_STA);
}

static esp_err_t ensure_ap_mode(void) {
    wifi_mode_t mode;
    esp_err_t   err = esp_wifi_get_mode(&mode);
    if (err != ESP_OK) {
        return err;
    }

    if (mode == WIFI_MODE_AP || mode == WIFI_MODE_APSTA) {
        return ESP_OK;
    }

    if (mode == WIFI_MODE_STA) {
        return esp_wifi_set_mode(WIFI_MODE_APSTA);
    }

    return esp_wifi_set_mode(WIFI_MODE_AP);
}

static void clear_scan_records(dom_models_wifi_scan_result_t* out) {
    if (!out) {
        return;
    }

    out->total_count = 0;
    out->count       = 0;
    out->truncated   = false;
    memset(out->records, 0, sizeof(out->records));
}

static void load_scan_records(inf_device_wifi_esp_wifi_impl_ctx_t* ctx) {
    if (!ctx) {
        return;
    }

    uint16_t  total = 0;
    esp_err_t err   = esp_wifi_scan_get_ap_num(&total);
    if (err != ESP_OK) {
        ctx->scanned.status        = DOM_MODELS_WIFI_SCAN_STATUS_FAILED;
        ctx->scanned.driver_status = (uint32_t)err;
        clear_scan_records(&ctx->scanned);
        return;
    }

    ctx->scanned.total_count = total;
    ctx->scanned.count       = total;
    if (ctx->scanned.count > DOM_MODELS_WIFI_SCAN_RESULT_MAX) {
        ctx->scanned.count     = DOM_MODELS_WIFI_SCAN_RESULT_MAX;
        ctx->scanned.truncated = true;
    }

    if (ctx->scanned.count == 0) {
        return;
    }

    wifi_ap_record_t records[DOM_MODELS_WIFI_SCAN_RESULT_MAX];
    memset(records, 0, sizeof(records));

    uint16_t count = (uint16_t)ctx->scanned.count;
    err            = esp_wifi_scan_get_ap_records(&count, records);
    if (err != ESP_OK) {
        ctx->scanned.status        = DOM_MODELS_WIFI_SCAN_STATUS_FAILED;
        ctx->scanned.driver_status = (uint32_t)err;
        clear_scan_records(&ctx->scanned);
        return;
    }

    ctx->scanned.count = count;
    for (size_t i = 0; i < ctx->scanned.count; i++) {
        copy_ap_record(&ctx->scanned.records[i], &records[i]);
    }
}
