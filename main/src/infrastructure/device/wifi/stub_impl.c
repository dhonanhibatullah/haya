#include "infrastructure/device/wifi/stub_impl.h"

#include <stdlib.h>
#include <string.h>

#include "domain/contracts/device/wifi.h"
#include "domain/models/error.h"
#include "domain/models/wifi.h"

#define STUB_WIFI_IMPL_ACTOR "wifi_stub"

/* Helper Function Prototypes */

static dom_models_error_t* bad_argument_error(const char* msg);
static size_t              bounded_strlen(const char* value, size_t max_len);
static void                copy_cstr(char* dst, size_t dst_size, const char* src);
static void                copy_if_key(bool* available, char* out, size_t out_size, const char* if_key);
static void                copy_mac(uint8_t dst[DOM_MODELS_WIFI_MAC_LEN], const uint8_t src[DOM_MODELS_WIFI_MAC_LEN]);
static const char*         default_ssid(inf_device_wifi_stub_impl_ctx_t* ctx);
static void                fill_ap_record(dom_models_wifi_ap_record_t* out, const char* ssid, const uint8_t bssid[DOM_MODELS_WIFI_MAC_LEN], uint8_t channel, int8_t rssi, dom_models_wifi_auth_mode_t auth_mode);
static void                fill_default_connected_ap(inf_device_wifi_stub_impl_ctx_t* ctx, const char* ssid, uint8_t channel, dom_models_wifi_auth_mode_t auth_mode);
static void                fill_default_scan(inf_device_wifi_stub_impl_ctx_t* ctx, const char* ssid, uint8_t channel);
static void                reset_runtime(inf_device_wifi_stub_impl_ctx_t* ctx);

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

dom_contracts_device_wifi_t* inf_device_wifi_stub_impl_new(const inf_device_wifi_stub_impl_cfg_t* cfg) {
    inf_device_wifi_stub_impl_ctx_t* ctx = (inf_device_wifi_stub_impl_ctx_t*)calloc(1, sizeof(inf_device_wifi_stub_impl_ctx_t));
    if (!ctx) {
        return NULL;
    }

    if (!cfg) {
        inf_device_wifi_stub_impl_cfg_t default_cfg = INF_DEVICE_WIFI_STUB_IMPL_CFG_DEFAULT();
        memcpy(&ctx->cfg, &default_cfg, sizeof(inf_device_wifi_stub_impl_cfg_t));
    } else {
        memcpy(&ctx->cfg, cfg, sizeof(inf_device_wifi_stub_impl_cfg_t));
    }

    ctx->mode = DOM_MODELS_WIFI_MODE_NULL;
    fill_default_connected_ap(ctx, default_ssid(ctx), 1, DOM_MODELS_WIFI_AUTH_WPA2_PSK);
    fill_default_scan(ctx, default_ssid(ctx), 1);

    dom_contracts_device_wifi_t* self = dom_contracts_device_wifi_new(ctx);
    if (!self) {
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

void inf_device_wifi_stub_impl_delete(dom_contracts_device_wifi_t* self) {
    if (!self || !self->ctx) {
        return;
    }

    inf_device_wifi_stub_impl_ctx_t* ctx = self->ctx;

    dom_models_error_t* err = inf_device_wifi_stub_impl_deinit(self);
    dom_models_error_delete(err);

    free(ctx);
    dom_contracts_device_wifi_delete(self);
}

dom_models_error_t* inf_device_wifi_stub_impl_init(dom_contracts_device_wifi_t* self) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing wifi stub context");
    }

    inf_device_wifi_stub_impl_ctx_t* ctx = self->ctx;
    ctx->initialized                    = true;

    return NULL;
}

dom_models_error_t* inf_device_wifi_stub_impl_deinit(dom_contracts_device_wifi_t* self) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing wifi stub context");
    }

    inf_device_wifi_stub_impl_ctx_t* ctx = self->ctx;

    reset_runtime(ctx);
    ctx->initialized = false;

    return NULL;
}

/* Contract Function Implementations */

static dom_models_error_t* start_impl(
    dom_contracts_device_wifi_t* self
) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing wifi stub context");
    }

    inf_device_wifi_stub_impl_ctx_t* ctx = self->ctx;
    ctx->started                        = true;

    if (ctx->mode == DOM_MODELS_WIFI_MODE_NULL) {
        ctx->mode = DOM_MODELS_WIFI_MODE_STA;
    }

    return NULL;
}

static dom_models_error_t* stop_impl(
    dom_contracts_device_wifi_t* self
) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing wifi stub context");
    }

    inf_device_wifi_stub_impl_ctx_t* ctx = self->ctx;

    reset_runtime(ctx);

    return NULL;
}

static dom_models_error_t* set_mode_impl(
    dom_contracts_device_wifi_t* self,
    dom_models_wifi_mode_t       mode
) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing wifi stub context");
    }

    inf_device_wifi_stub_impl_ctx_t* ctx = self->ctx;
    ctx->mode                           = mode;

    return NULL;
}

static dom_models_error_t* get_status_impl(
    dom_contracts_device_wifi_t* self,
    dom_models_wifi_status_t*    out
) {
    if (!self || !self->ctx || !out) {
        return bad_argument_error("missing wifi stub status argument");
    }

    static const uint8_t sta_mac[DOM_MODELS_WIFI_MAC_LEN]    = {0x02, 0x00, 0x00, 0x00, 0x00, 0x01};
    static const uint8_t ap_mac[DOM_MODELS_WIFI_MAC_LEN]     = {0x02, 0x00, 0x00, 0x00, 0x00, 0x02};
    static const uint8_t client_mac[DOM_MODELS_WIFI_MAC_LEN] = {0x02, 0x00, 0x00, 0x00, 0x20, 0x01};

    inf_device_wifi_stub_impl_ctx_t* ctx = self->ctx;

    memset(out, 0, sizeof(dom_models_wifi_status_t));

    out->mode               = ctx->mode;
    out->started            = ctx->started;
    out->connected          = ctx->connected;
    out->sta_mac_available  = true;
    out->ap_mac_available   = true;

    copy_if_key(&out->sta_if_key_available, out->sta_if_key, sizeof(out->sta_if_key), ctx->cfg.sta_if_key);
    copy_if_key(&out->ap_if_key_available, out->ap_if_key, sizeof(out->ap_if_key), ctx->cfg.ap_if_key);
    copy_mac(out->sta_mac, sta_mac);
    copy_mac(out->ap_mac, ap_mac);

    if (ctx->connected) {
        out->connected_ap_available = true;
        memcpy(&out->connected_ap, &ctx->connected_ap, sizeof(dom_models_wifi_ap_record_t));
    }

    if (ctx->ap_started) {
        out->ap_client_total_count = 1;
        out->ap_client_count       = 1;
        copy_mac(out->ap_clients[0].mac, client_mac);
        out->ap_clients[0].rssi      = -37;
        out->ap_clients[0].phy_flags = DOM_MODELS_WIFI_PHY_FLAG_11B | DOM_MODELS_WIFI_PHY_FLAG_11G | DOM_MODELS_WIFI_PHY_FLAG_11N;
    }

    return NULL;
}

static dom_models_error_t* connect_sta_impl(
    dom_contracts_device_wifi_t*                self,
    const dom_models_wifi_sta_connect_config_t* config
) {
    if (!self || !self->ctx || !config) {
        return bad_argument_error("missing wifi stub sta config");
    }

    inf_device_wifi_stub_impl_ctx_t* ctx = self->ctx;
    const char*                      ssid = bounded_strlen(config->ssid, sizeof(config->ssid)) > 0 ? config->ssid : default_ssid(ctx);
    uint8_t                          channel = config->channel_set ? config->channel : 1;

    ctx->started   = true;
    ctx->connected = true;

    if (ctx->mode == DOM_MODELS_WIFI_MODE_AP || ctx->ap_started) {
        ctx->mode = DOM_MODELS_WIFI_MODE_APSTA;
    } else {
        ctx->mode = DOM_MODELS_WIFI_MODE_STA;
    }

    fill_default_connected_ap(ctx, ssid, channel, DOM_MODELS_WIFI_AUTH_WPA2_PSK);

    return NULL;
}

static dom_models_error_t* disconnect_sta_impl(
    dom_contracts_device_wifi_t* self
) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing wifi stub context");
    }

    inf_device_wifi_stub_impl_ctx_t* ctx = self->ctx;
    ctx->connected                      = false;

    if (ctx->mode == DOM_MODELS_WIFI_MODE_APSTA) {
        ctx->mode = DOM_MODELS_WIFI_MODE_AP;
    }

    return NULL;
}

static dom_models_error_t* start_ap_impl(
    dom_contracts_device_wifi_t*       self,
    const dom_models_wifi_ap_config_t* config
) {
    if (!self || !self->ctx || !config) {
        return bad_argument_error("missing wifi stub ap config");
    }

    inf_device_wifi_stub_impl_ctx_t* ctx = self->ctx;
    const char*                      ssid = bounded_strlen(config->ssid, sizeof(config->ssid)) > 0 ? config->ssid : default_ssid(ctx);
    uint8_t                          channel = config->channel > 0 ? config->channel : 1;
    dom_models_wifi_auth_mode_t      auth_mode = config->auth_mode == DOM_MODELS_WIFI_AUTH_UNKNOWN ? DOM_MODELS_WIFI_AUTH_WPA2_PSK : config->auth_mode;

    ctx->started    = true;
    ctx->ap_started = true;

    if (ctx->mode == DOM_MODELS_WIFI_MODE_STA || ctx->connected) {
        ctx->mode = DOM_MODELS_WIFI_MODE_APSTA;
    } else {
        ctx->mode = DOM_MODELS_WIFI_MODE_AP;
    }

    fill_default_connected_ap(ctx, ssid, channel, auth_mode);

    return NULL;
}

static dom_models_error_t* stop_ap_impl(
    dom_contracts_device_wifi_t* self
) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing wifi stub context");
    }

    inf_device_wifi_stub_impl_ctx_t* ctx = self->ctx;
    ctx->ap_started                     = false;

    if (ctx->mode == DOM_MODELS_WIFI_MODE_APSTA) {
        ctx->mode = DOM_MODELS_WIFI_MODE_STA;
    } else if (ctx->mode == DOM_MODELS_WIFI_MODE_AP) {
        ctx->mode    = DOM_MODELS_WIFI_MODE_NULL;
        ctx->started = false;
    }

    return NULL;
}

static dom_models_error_t* start_scan_impl(
    dom_contracts_device_wifi_t*         self,
    const dom_models_wifi_scan_config_t* config
) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing wifi stub context");
    }

    inf_device_wifi_stub_impl_ctx_t* ctx = self->ctx;
    const char*                      ssid = default_ssid(ctx);
    uint8_t                          channel = 1;

    if (config) {
        if (config->ssid_set && bounded_strlen(config->ssid, sizeof(config->ssid)) > 0) {
            ssid = config->ssid;
        }
        if (config->channel_set) {
            channel = config->channel;
        }
    }

    fill_default_scan(ctx, ssid, channel);
    ctx->scanned.scan_id += 1;

    return NULL;
}

static dom_models_error_t* get_scanned_impl(
    dom_contracts_device_wifi_t*   self,
    dom_models_wifi_scan_result_t* out
) {
    if (!self || !self->ctx || !out) {
        return bad_argument_error("missing wifi stub scan result argument");
    }

    inf_device_wifi_stub_impl_ctx_t* ctx = self->ctx;
    memcpy(out, &ctx->scanned, sizeof(dom_models_wifi_scan_result_t));

    return NULL;
}

/* Helper Function Implementations */

static dom_models_error_t* bad_argument_error(const char* msg) {
    return dom_models_error_new(STUB_WIFI_IMPL_ACTOR, DOMAIN_MODELS_ERROR_TYPE_BAD_ARGUMENT, "%s", msg ? msg : "bad argument");
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

static void copy_mac(uint8_t dst[DOM_MODELS_WIFI_MAC_LEN], const uint8_t src[DOM_MODELS_WIFI_MAC_LEN]) {
    if (!dst || !src) {
        return;
    }

    memcpy(dst, src, DOM_MODELS_WIFI_MAC_LEN);
}

static const char* default_ssid(inf_device_wifi_stub_impl_ctx_t* ctx) {
    if (!ctx || !ctx->cfg.default_ssid || ctx->cfg.default_ssid[0] == '\0') {
        return "haya-stub";
    }

    return ctx->cfg.default_ssid;
}

static void fill_ap_record(dom_models_wifi_ap_record_t* out, const char* ssid, const uint8_t bssid[DOM_MODELS_WIFI_MAC_LEN], uint8_t channel, int8_t rssi, dom_models_wifi_auth_mode_t auth_mode) {
    if (!out) {
        return;
    }

    memset(out, 0, sizeof(dom_models_wifi_ap_record_t));

    out->bssid_available = true;
    copy_mac(out->bssid, bssid);
    copy_cstr(out->ssid, sizeof(out->ssid), ssid);
    out->primary_channel = channel;
    out->second_channel  = DOM_MODELS_WIFI_SECOND_CHANNEL_NONE;
    out->rssi            = rssi;
    out->auth_mode       = auth_mode;
    out->pairwise_cipher = DOM_MODELS_WIFI_CIPHER_CCMP;
    out->group_cipher    = DOM_MODELS_WIFI_CIPHER_CCMP;
    out->bandwidth       = DOM_MODELS_WIFI_BANDWIDTH_20MHZ;
    out->phy_flags       = DOM_MODELS_WIFI_PHY_FLAG_11B | DOM_MODELS_WIFI_PHY_FLAG_11G | DOM_MODELS_WIFI_PHY_FLAG_11N;
}

static void fill_default_connected_ap(inf_device_wifi_stub_impl_ctx_t* ctx, const char* ssid, uint8_t channel, dom_models_wifi_auth_mode_t auth_mode) {
    static const uint8_t bssid[DOM_MODELS_WIFI_MAC_LEN] = {0x02, 0x00, 0x00, 0x00, 0x10, 0x01};

    if (!ctx) {
        return;
    }

    fill_ap_record(&ctx->connected_ap, ssid, bssid, channel, -42, auth_mode);
}

static void fill_default_scan(inf_device_wifi_stub_impl_ctx_t* ctx, const char* ssid, uint8_t channel) {
    static const uint8_t bssid[DOM_MODELS_WIFI_MAC_LEN] = {0x02, 0x00, 0x00, 0x00, 0x10, 0x01};
    uint32_t             scan_id = 0;

    if (!ctx) {
        return;
    }

    scan_id = ctx->scanned.scan_id;

    memset(&ctx->scanned, 0, sizeof(dom_models_wifi_scan_result_t));

    ctx->scanned.status        = DOM_MODELS_WIFI_SCAN_STATUS_DONE;
    ctx->scanned.scan_id       = scan_id;
    ctx->scanned.driver_status = 0;
    ctx->scanned.total_count   = 1;
    ctx->scanned.count         = 1;
    ctx->scanned.truncated     = false;

    fill_ap_record(&ctx->scanned.records[0], ssid, bssid, channel, -42, DOM_MODELS_WIFI_AUTH_WPA2_PSK);
}

static void reset_runtime(inf_device_wifi_stub_impl_ctx_t* ctx) {
    if (!ctx) {
        return;
    }

    ctx->started      = false;
    ctx->connected    = false;
    ctx->ap_started   = false;
    ctx->mode         = DOM_MODELS_WIFI_MODE_NULL;

    fill_default_connected_ap(ctx, default_ssid(ctx), 1, DOM_MODELS_WIFI_AUTH_WPA2_PSK);
    fill_default_scan(ctx, default_ssid(ctx), 1);
}
