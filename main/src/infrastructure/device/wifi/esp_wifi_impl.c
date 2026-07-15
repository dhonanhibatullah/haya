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
#include "infrastructure/device/wifi/esp_wifi_impl_utils.h"

/* Event Handler Function Prototypes */

static void wifi_event_handler(void* arg, esp_event_base_t base, int32_t id, void* data);
static void on_scan_done(inf_device_wifi_esp_wifi_impl_ctx_t* ctx, const wifi_event_sta_scan_done_t* event);
static void on_sta_connected(inf_device_wifi_esp_wifi_impl_ctx_t* ctx);
static void on_sta_disconnected(inf_device_wifi_esp_wifi_impl_ctx_t* ctx, const wifi_event_sta_disconnected_t* event);
static void on_ap_start(inf_device_wifi_esp_wifi_impl_ctx_t* ctx);
static void on_ap_stop(inf_device_wifi_esp_wifi_impl_ctx_t* ctx);

/* Helper Function Prototypes */

static void dispatch_event(
    inf_device_wifi_esp_wifi_impl_ctx_t* ctx,
    dom_models_wifi_event_type_t         type,
    uint32_t                             driver_status
);

/* Contract Function Prototypes */

static dom_models_error_t start_impl(
    dom_contracts_device_wifi_t* self
);
static dom_models_error_t stop_impl(
    dom_contracts_device_wifi_t* self
);
static dom_models_error_t set_mode_impl(
    dom_contracts_device_wifi_t* self,
    dom_models_wifi_mode_t       mode
);
static dom_models_error_t get_status_impl(
    dom_contracts_device_wifi_t* self,
    dom_models_wifi_status_t*    out
);
static dom_models_error_t connect_sta_impl(
    dom_contracts_device_wifi_t*                self,
    const dom_models_wifi_sta_connect_config_t* config
);
static dom_models_error_t disconnect_sta_impl(
    dom_contracts_device_wifi_t* self
);
static dom_models_error_t start_ap_impl(
    dom_contracts_device_wifi_t*       self,
    const dom_models_wifi_ap_config_t* config
);
static dom_models_error_t stop_ap_impl(
    dom_contracts_device_wifi_t* self
);
static dom_models_error_t start_scan_impl(
    dom_contracts_device_wifi_t*         self,
    const dom_models_wifi_scan_config_t* config
);
static dom_models_error_t get_scanned_impl(
    dom_contracts_device_wifi_t*   self,
    dom_models_wifi_scan_result_t* out
);
static dom_models_error_t add_event_callback_impl(
    dom_contracts_device_wifi_t*     self,
    void*                            cb_ctx,
    dom_models_wifi_event_callback_t cb_func
);
static dom_models_error_t remove_event_callback_impl(
    dom_contracts_device_wifi_t*     self,
    dom_models_wifi_event_callback_t cb_func
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

    dom_contracts_device_wifi_t* self = dom_contracts_device_wifi_new(ctx);
    if (!self) {
        free(ctx);
        return NULL;
    }

    self->start                 = start_impl;
    self->stop                  = stop_impl;
    self->set_mode              = set_mode_impl;
    self->get_status            = get_status_impl;
    self->connect_sta           = connect_sta_impl;
    self->disconnect_sta        = disconnect_sta_impl;
    self->start_ap              = start_ap_impl;
    self->stop_ap               = stop_ap_impl;
    self->start_scan            = start_scan_impl;
    self->get_scanned           = get_scanned_impl;
    self->add_event_callback    = add_event_callback_impl;
    self->remove_event_callback = remove_event_callback_impl;

    return self;
}

void inf_device_wifi_esp_wifi_impl_delete(dom_contracts_device_wifi_t* self) {
    if (!self) {
        return;
    }

    inf_device_wifi_esp_wifi_impl_ctx_t* ctx = self->ctx;
    if (ctx) {
        (void)inf_device_wifi_esp_wifi_impl_deinit(self);
        free(ctx);
    }

    dom_contracts_device_wifi_delete(self);
}

dom_models_error_t inf_device_wifi_esp_wifi_impl_init(dom_contracts_device_wifi_t* self) {
    if (!self || !self->ctx) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_device_wifi_esp_wifi_impl_ctx_t* ctx = self->ctx;

    if (ctx->initialized) {
        return DOMAIN_MODELS_ERROR_OK;
    }

    if (ctx->cfg.register_event_handler) {
        esp_err_t err = esp_event_handler_instance_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            wifi_event_handler,
            ctx,
            &ctx->wifi_event_handler
        );
        if (err != ESP_OK) {
            return inf_device_wifi_esp_wifi_impl_error_from_esp(err);
        }
        ctx->wifi_event_handler_registered = true;
    }

    ctx->initialized = true;

    return DOMAIN_MODELS_ERROR_OK;
}

dom_models_error_t inf_device_wifi_esp_wifi_impl_deinit(dom_contracts_device_wifi_t* self) {
    if (!self || !self->ctx) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_device_wifi_esp_wifi_impl_ctx_t* ctx    = self->ctx;
    dom_models_error_t                   result = DOMAIN_MODELS_ERROR_OK;

    if (ctx->started) {
        esp_err_t err = esp_wifi_stop();
        if (err != ESP_OK && err != ESP_ERR_WIFI_NOT_STARTED && result == DOMAIN_MODELS_ERROR_OK) {
            result = inf_device_wifi_esp_wifi_impl_error_from_esp(err);
        }
    }

    if (ctx->wifi_event_handler_registered) {
        esp_err_t err = esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, ctx->wifi_event_handler);
        if (err != ESP_OK) {
            if (result == DOMAIN_MODELS_ERROR_OK) {
                result = inf_device_wifi_esp_wifi_impl_error_from_esp(err);
            }
        } else {
            ctx->wifi_event_handler_registered = false;
            ctx->wifi_event_handler            = NULL;
        }
    }

    ctx->initialized   = false;
    ctx->started       = false;
    ctx->sta_connected = false;
    ctx->ap_started    = false;
    memset(&ctx->scanned, 0, sizeof(dom_models_wifi_scan_result_t));
    ctx->scanned.status = DOM_MODELS_WIFI_SCAN_STATUS_IDLE;

    return result;
}

/* Contract Function Implementations */

static dom_models_error_t start_impl(
    dom_contracts_device_wifi_t* self
) {
    if (!self || !self->ctx) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_device_wifi_esp_wifi_impl_ctx_t* ctx = self->ctx;

    esp_err_t err = esp_wifi_start();
    if (err != ESP_OK) {
        return inf_device_wifi_esp_wifi_impl_error_from_esp(err);
    }

    ctx->started = true;

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t stop_impl(
    dom_contracts_device_wifi_t* self
) {
    if (!self || !self->ctx) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_device_wifi_esp_wifi_impl_ctx_t* ctx = self->ctx;

    esp_err_t err = esp_wifi_stop();
    if (err != ESP_OK) {
        return inf_device_wifi_esp_wifi_impl_error_from_esp(err);
    }

    ctx->started       = false;
    ctx->sta_connected = false;
    ctx->ap_started    = false;
    memset(&ctx->scanned, 0, sizeof(dom_models_wifi_scan_result_t));
    ctx->scanned.status = DOM_MODELS_WIFI_SCAN_STATUS_IDLE;

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t set_mode_impl(
    dom_contracts_device_wifi_t* self,
    dom_models_wifi_mode_t       mode
) {
    if (!self || !self->ctx) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    wifi_mode_t wifi_mode;
    if (!inf_device_wifi_esp_wifi_impl_wifi_mode_from_domain(mode, &wifi_mode)) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    esp_err_t err = esp_wifi_set_mode(wifi_mode);
    if (err != ESP_OK) {
        return inf_device_wifi_esp_wifi_impl_error_from_esp(err);
    }

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t get_status_impl(
    dom_contracts_device_wifi_t* self,
    dom_models_wifi_status_t*    out
) {
    if (!self || !self->ctx || !out) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_device_wifi_esp_wifi_impl_ctx_t* ctx = self->ctx;

    memset(out, 0, sizeof(dom_models_wifi_status_t));

    wifi_mode_t mode;
    esp_err_t   err = esp_wifi_get_mode(&mode);
    if (err != ESP_OK) {
        return inf_device_wifi_esp_wifi_impl_error_from_esp(err);
    }

    out->mode      = inf_device_wifi_esp_wifi_impl_wifi_mode_to_domain(mode);
    out->started   = ctx->started;
    out->connected = ctx->sta_connected;

    inf_device_wifi_esp_wifi_impl_copy_if_key(&out->sta_if_key_available, out->sta_if_key, sizeof(out->sta_if_key), ctx->cfg.sta_if_key);
    inf_device_wifi_esp_wifi_impl_copy_if_key(&out->ap_if_key_available, out->ap_if_key, sizeof(out->ap_if_key), ctx->cfg.ap_if_key);

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
        inf_device_wifi_esp_wifi_impl_copy_ap_record(&out->connected_ap, &ap_record);
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
            inf_device_wifi_esp_wifi_impl_copy_ap_client(&out->ap_clients[i], &sta_list.sta[i]);
        }
    }

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t connect_sta_impl(
    dom_contracts_device_wifi_t*                self,
    const dom_models_wifi_sta_connect_config_t* config
) {
    if (!self || !self->ctx || !config) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    if (inf_device_wifi_esp_wifi_impl_bounded_strlen(config->ssid, sizeof(config->ssid)) == 0) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }
    if (config->channel_set && config->channel == 0) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    esp_err_t err = inf_device_wifi_esp_wifi_impl_ensure_sta_mode();
    if (err != ESP_OK) {
        return inf_device_wifi_esp_wifi_impl_error_from_esp(err);
    }

    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config_t));

    inf_device_wifi_esp_wifi_impl_copy_cstr_to_bytes(wifi_config.sta.ssid, sizeof(wifi_config.sta.ssid), config->ssid);
    inf_device_wifi_esp_wifi_impl_copy_cstr_to_bytes(wifi_config.sta.password, sizeof(wifi_config.sta.password), config->password);
    wifi_config.sta.bssid_set = config->bssid_set;
    if (config->bssid_set) {
        memcpy(wifi_config.sta.bssid, config->bssid, sizeof(wifi_config.sta.bssid));
    }
    if (config->channel_set) {
        wifi_config.sta.channel = config->channel;
    }

    err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (err != ESP_OK) {
        return inf_device_wifi_esp_wifi_impl_error_from_esp(err);
    }

    err = esp_wifi_connect();
    if (err != ESP_OK) {
        return inf_device_wifi_esp_wifi_impl_error_from_esp(err);
    }

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t disconnect_sta_impl(
    dom_contracts_device_wifi_t* self
) {
    if (!self || !self->ctx) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    esp_err_t err = esp_wifi_disconnect();
    if (err != ESP_OK) {
        return inf_device_wifi_esp_wifi_impl_error_from_esp(err);
    }

    inf_device_wifi_esp_wifi_impl_ctx_t* ctx = self->ctx;
    ctx->sta_connected                       = false;

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t start_ap_impl(
    dom_contracts_device_wifi_t*       self,
    const dom_models_wifi_ap_config_t* config
) {
    if (!self || !self->ctx || !config) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    if (inf_device_wifi_esp_wifi_impl_bounded_strlen(config->ssid, sizeof(config->ssid)) == 0) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }
    if (config->channel_set && config->channel == 0) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }
    if (config->max_clients_set && config->max_clients == 0) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    dom_models_wifi_auth_mode_t domain_auth_mode = config->auth_mode_set ? config->auth_mode : DOM_MODELS_WIFI_AP_DEFAULT_AUTH_MODE;
    wifi_auth_mode_t            auth_mode;
    if (!inf_device_wifi_esp_wifi_impl_wifi_auth_from_domain(domain_auth_mode, &auth_mode)) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    esp_err_t err = inf_device_wifi_esp_wifi_impl_ensure_ap_mode();
    if (err != ESP_OK) {
        return inf_device_wifi_esp_wifi_impl_error_from_esp(err);
    }

    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config_t));

    inf_device_wifi_esp_wifi_impl_copy_cstr_to_bytes(wifi_config.ap.ssid, sizeof(wifi_config.ap.ssid), config->ssid);
    inf_device_wifi_esp_wifi_impl_copy_cstr_to_bytes(wifi_config.ap.password, sizeof(wifi_config.ap.password), config->password);
    wifi_config.ap.ssid_len       = (uint8_t)inf_device_wifi_esp_wifi_impl_bounded_strlen(config->ssid, sizeof(wifi_config.ap.ssid));
    wifi_config.ap.channel        = config->channel_set ? config->channel : DOM_MODELS_WIFI_AP_DEFAULT_CHANNEL;
    wifi_config.ap.authmode       = auth_mode;
    wifi_config.ap.max_connection = config->max_clients_set ? config->max_clients : DOM_MODELS_WIFI_AP_DEFAULT_MAX_CLIENTS;
    wifi_config.ap.ssid_hidden    = config->hidden ? 1 : 0;

    err = esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    if (err != ESP_OK) {
        return inf_device_wifi_esp_wifi_impl_error_from_esp(err);
    }

    inf_device_wifi_esp_wifi_impl_ctx_t* ctx = self->ctx;
    if (!ctx->started) {
        err = esp_wifi_start();
        if (err != ESP_OK) {
            return inf_device_wifi_esp_wifi_impl_error_from_esp(err);
        }
        ctx->started = true;
    }

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t stop_ap_impl(
    dom_contracts_device_wifi_t* self
) {
    if (!self || !self->ctx) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_device_wifi_esp_wifi_impl_ctx_t* ctx = self->ctx;

    wifi_mode_t mode;
    esp_err_t   err = esp_wifi_get_mode(&mode);
    if (err != ESP_OK) {
        return inf_device_wifi_esp_wifi_impl_error_from_esp(err);
    }

    if (mode == WIFI_MODE_APSTA) {
        err = esp_wifi_set_mode(WIFI_MODE_STA);
        if (err != ESP_OK) {
            return inf_device_wifi_esp_wifi_impl_error_from_esp(err);
        }
        ctx->ap_started = false;
        return DOMAIN_MODELS_ERROR_OK;
    }

    if (mode == WIFI_MODE_AP) {
        err = esp_wifi_stop();
        if (err != ESP_OK) {
            return inf_device_wifi_esp_wifi_impl_error_from_esp(err);
        }
        ctx->started    = false;
        ctx->ap_started = false;
    }

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t start_scan_impl(
    dom_contracts_device_wifi_t*         self,
    const dom_models_wifi_scan_config_t* config
) {
    if (!self || !self->ctx) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_device_wifi_esp_wifi_impl_ctx_t* ctx = self->ctx;

    memset(&ctx->scanned, 0, sizeof(dom_models_wifi_scan_result_t));
    ctx->scanned.status = DOM_MODELS_WIFI_SCAN_STATUS_RUNNING;

    wifi_scan_config_t scan_config;
    memset(&scan_config, 0, sizeof(wifi_scan_config_t));

    uint8_t ssid[DOM_MODELS_WIFI_SSID_BUF_LEN];
    uint8_t bssid[DOM_MODELS_WIFI_MAC_LEN];
    memset(ssid, 0, sizeof(ssid));
    memset(bssid, 0, sizeof(bssid));

    if (config) {
        if (config->ssid_set) {
            inf_device_wifi_esp_wifi_impl_copy_cstr_to_bytes(ssid, sizeof(ssid), config->ssid);
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
        return inf_device_wifi_esp_wifi_impl_error_from_esp(err);
    }

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t get_scanned_impl(
    dom_contracts_device_wifi_t*   self,
    dom_models_wifi_scan_result_t* out
) {
    if (!self || !self->ctx || !out) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_device_wifi_esp_wifi_impl_ctx_t* ctx = self->ctx;
    memcpy(out, &ctx->scanned, sizeof(dom_models_wifi_scan_result_t));

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t add_event_callback_impl(
    dom_contracts_device_wifi_t*     self,
    void*                            cb_ctx,
    dom_models_wifi_event_callback_t cb_func
) {
    if (!self || !self->ctx || !cb_func) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_device_wifi_esp_wifi_impl_ctx_t* ctx = self->ctx;

    for (size_t i = 0; i < ctx->event_cb_cnt; i++) {
        if (ctx->event_cb_funcs[i] == cb_func) {
            return DOMAIN_MODELS_ERROR_OK;
        }
    }

    if (ctx->event_cb_cnt >= INF_DEVICE_WIFI_ESP_WIFI_IMPL_EVENT_CALLBACK_MAX) {
        return DOMAIN_MODELS_ERROR_BAD_STATE;
    }

    ctx->event_cb_funcs[ctx->event_cb_cnt] = cb_func;
    ctx->event_cb_ctxs[ctx->event_cb_cnt]  = cb_ctx;
    ctx->event_cb_cnt += 1;

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t remove_event_callback_impl(
    dom_contracts_device_wifi_t*     self,
    dom_models_wifi_event_callback_t cb_func
) {
    if (!self || !self->ctx || !cb_func) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_device_wifi_esp_wifi_impl_ctx_t* ctx = self->ctx;

    for (size_t i = 0; i < ctx->event_cb_cnt; i++) {
        if (ctx->event_cb_funcs[i] != cb_func) {
            continue;
        }

        size_t last_idx = ctx->event_cb_cnt - 1;

        ctx->event_cb_funcs[i] = NULL;
        ctx->event_cb_ctxs[i]  = NULL;

        if (i != last_idx) {
            ctx->event_cb_funcs[i] = ctx->event_cb_funcs[last_idx];
            ctx->event_cb_ctxs[i]  = ctx->event_cb_ctxs[last_idx];

            ctx->event_cb_funcs[last_idx] = NULL;
            ctx->event_cb_ctxs[last_idx]  = NULL;
        }

        ctx->event_cb_cnt -= 1;

        return DOMAIN_MODELS_ERROR_OK;
    }

    return DOMAIN_MODELS_ERROR_NOT_FOUND;
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
            on_sta_disconnected(ctx, (const wifi_event_sta_disconnected_t*)data);
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

    inf_device_wifi_esp_wifi_impl_clear_scan_records(&ctx->scanned);

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
    inf_device_wifi_esp_wifi_impl_load_scan_records(ctx);
}

static void on_sta_connected(inf_device_wifi_esp_wifi_impl_ctx_t* ctx) {
    if (!ctx) {
        return;
    }

    ctx->sta_connected = true;
    dispatch_event(ctx, DOM_MODELS_WIFI_EVENT_STA_CONNECTED, 0);
}

static void on_sta_disconnected(inf_device_wifi_esp_wifi_impl_ctx_t* ctx, const wifi_event_sta_disconnected_t* event) {
    if (!ctx) {
        return;
    }

    ctx->sta_connected = false;
    dispatch_event(ctx, DOM_MODELS_WIFI_EVENT_STA_DISCONNECTED, event ? (uint32_t)event->reason : 0);
}

static void on_ap_start(inf_device_wifi_esp_wifi_impl_ctx_t* ctx) {
    if (!ctx) {
        return;
    }

    ctx->ap_started = true;
    ctx->started    = true;
    dispatch_event(ctx, DOM_MODELS_WIFI_EVENT_AP_STARTED, 0);
}

static void on_ap_stop(inf_device_wifi_esp_wifi_impl_ctx_t* ctx) {
    if (!ctx) {
        return;
    }

    ctx->ap_started = false;
    dispatch_event(ctx, DOM_MODELS_WIFI_EVENT_AP_STOPPED, 0);
}

/* Helper Function Implementations */

static void dispatch_event(
    inf_device_wifi_esp_wifi_impl_ctx_t* ctx,
    dom_models_wifi_event_type_t         type,
    uint32_t                             driver_status
) {
    if (!ctx) {
        return;
    }

    dom_models_wifi_event_t event = {
        .type          = type,
        .driver_status = driver_status,
    };

    size_t cb_cnt = ctx->event_cb_cnt;
    for (size_t i = 0; i < cb_cnt; i++) {
        if (!ctx->event_cb_funcs[i]) {
            continue;
        }

        ctx->event_cb_funcs[i](ctx->event_cb_ctxs[i], &event);
    }
}
