#include "infrastructure/device/wifi/stub_impl.h"

#include <stdlib.h>
#include <string.h>

#include "domain/contracts/device/wifi.h"
#include "domain/models/error.h"
#include "domain/models/wifi.h"
#include "infrastructure/device/wifi/stub_impl_utils.h"

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
    inf_device_wifi_stub_impl_fill_default_connected_ap(ctx, inf_device_wifi_stub_impl_default_ssid(ctx), 1, DOM_MODELS_WIFI_AUTH_WPA2_PSK);
    inf_device_wifi_stub_impl_fill_default_scan(ctx, inf_device_wifi_stub_impl_default_ssid(ctx), 1);

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
    if (!self) {
        return;
    }

    inf_device_wifi_stub_impl_ctx_t* ctx = self->ctx;
    if (ctx) {
        (void)inf_device_wifi_stub_impl_deinit(self);
        free(ctx);
    }

    dom_contracts_device_wifi_delete(self);
}

dom_models_error_t inf_device_wifi_stub_impl_init(dom_contracts_device_wifi_t* self) {
    if (!self || !self->ctx) {
        return inf_device_wifi_stub_impl_bad_argument_error("missing wifi stub context");
    }

    inf_device_wifi_stub_impl_ctx_t* ctx = self->ctx;
    ctx->initialized                    = true;

    return DOMAIN_MODELS_ERROR_OK;
}

dom_models_error_t inf_device_wifi_stub_impl_deinit(dom_contracts_device_wifi_t* self) {
    if (!self || !self->ctx) {
        return inf_device_wifi_stub_impl_bad_argument_error("missing wifi stub context");
    }

    inf_device_wifi_stub_impl_ctx_t* ctx = self->ctx;

    inf_device_wifi_stub_impl_reset_runtime(ctx);
    ctx->initialized = false;

    return DOMAIN_MODELS_ERROR_OK;
}

/* Contract Function Implementations */

static dom_models_error_t start_impl(
    dom_contracts_device_wifi_t* self
) {
    if (!self || !self->ctx) {
        return inf_device_wifi_stub_impl_bad_argument_error("missing wifi stub context");
    }

    inf_device_wifi_stub_impl_ctx_t* ctx = self->ctx;
    ctx->started                        = true;

    if (ctx->mode == DOM_MODELS_WIFI_MODE_NULL) {
        ctx->mode = DOM_MODELS_WIFI_MODE_STA;
    }

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t stop_impl(
    dom_contracts_device_wifi_t* self
) {
    if (!self || !self->ctx) {
        return inf_device_wifi_stub_impl_bad_argument_error("missing wifi stub context");
    }

    inf_device_wifi_stub_impl_ctx_t* ctx = self->ctx;

    inf_device_wifi_stub_impl_reset_runtime(ctx);

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t set_mode_impl(
    dom_contracts_device_wifi_t* self,
    dom_models_wifi_mode_t       mode
) {
    if (!self || !self->ctx) {
        return inf_device_wifi_stub_impl_bad_argument_error("missing wifi stub context");
    }
    if (!inf_device_wifi_stub_impl_valid_mode(mode)) {
        return inf_device_wifi_stub_impl_bad_argument_error("invalid wifi stub mode");
    }

    inf_device_wifi_stub_impl_ctx_t* ctx = self->ctx;
    ctx->mode                           = mode;

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t get_status_impl(
    dom_contracts_device_wifi_t* self,
    dom_models_wifi_status_t*    out
) {
    if (!self || !self->ctx || !out) {
        return inf_device_wifi_stub_impl_bad_argument_error("missing wifi stub status argument");
    }

    static const uint8_t sta_mac[DOM_MODELS_WIFI_MAC_LEN]    = {0x02, 0x00, 0x00, 0x00, 0x00, 0x01};
    static const uint8_t ap_mac[DOM_MODELS_WIFI_MAC_LEN]     = {0x02, 0x00, 0x00, 0x00, 0x00, 0x02};
    static const uint8_t client_mac[DOM_MODELS_WIFI_MAC_LEN] = {0x02, 0x00, 0x00, 0x00, 0x20, 0x01};

    inf_device_wifi_stub_impl_ctx_t* ctx = self->ctx;

    memset(out, 0, sizeof(dom_models_wifi_status_t));

    out->mode              = ctx->mode;
    out->started           = ctx->started;
    out->connected         = ctx->connected;
    out->sta_mac_available = true;
    out->ap_mac_available  = true;

    inf_device_wifi_stub_impl_copy_if_key(&out->sta_if_key_available, out->sta_if_key, sizeof(out->sta_if_key), ctx->cfg.sta_if_key);
    inf_device_wifi_stub_impl_copy_if_key(&out->ap_if_key_available, out->ap_if_key, sizeof(out->ap_if_key), ctx->cfg.ap_if_key);
    inf_device_wifi_stub_impl_copy_mac(out->sta_mac, sta_mac);
    inf_device_wifi_stub_impl_copy_mac(out->ap_mac, ap_mac);

    if (ctx->connected) {
        out->connected_ap_available = true;
        memcpy(&out->connected_ap, &ctx->connected_ap, sizeof(dom_models_wifi_ap_record_t));
    }

    if (ctx->ap_started) {
        out->ap_client_total_count = 1;
        out->ap_client_count       = 1;
        inf_device_wifi_stub_impl_copy_mac(out->ap_clients[0].mac, client_mac);
        out->ap_clients[0].rssi      = -37;
        out->ap_clients[0].phy_flags = DOM_MODELS_WIFI_PHY_FLAG_11B | DOM_MODELS_WIFI_PHY_FLAG_11G | DOM_MODELS_WIFI_PHY_FLAG_11N;
    }

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t connect_sta_impl(
    dom_contracts_device_wifi_t*                self,
    const dom_models_wifi_sta_connect_config_t* config
) {
    if (!self || !self->ctx || !config) {
        return inf_device_wifi_stub_impl_bad_argument_error("missing wifi stub sta config");
    }

    inf_device_wifi_stub_impl_ctx_t* ctx     = self->ctx;
    const char*                      ssid    = inf_device_wifi_stub_impl_bounded_strlen(config->ssid, sizeof(config->ssid)) > 0 ? config->ssid : inf_device_wifi_stub_impl_default_ssid(ctx);
    uint8_t                          channel = config->channel_set ? config->channel : 1;

    ctx->started   = true;
    ctx->connected = true;

    if (ctx->mode == DOM_MODELS_WIFI_MODE_AP || ctx->ap_started) {
        ctx->mode = DOM_MODELS_WIFI_MODE_APSTA;
    } else {
        ctx->mode = DOM_MODELS_WIFI_MODE_STA;
    }

    inf_device_wifi_stub_impl_fill_default_connected_ap(ctx, ssid, channel, DOM_MODELS_WIFI_AUTH_WPA2_PSK);

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t disconnect_sta_impl(
    dom_contracts_device_wifi_t* self
) {
    if (!self || !self->ctx) {
        return inf_device_wifi_stub_impl_bad_argument_error("missing wifi stub context");
    }

    inf_device_wifi_stub_impl_ctx_t* ctx = self->ctx;
    ctx->connected                      = false;

    if (ctx->mode == DOM_MODELS_WIFI_MODE_APSTA) {
        ctx->mode = DOM_MODELS_WIFI_MODE_AP;
    }

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t start_ap_impl(
    dom_contracts_device_wifi_t*       self,
    const dom_models_wifi_ap_config_t* config
) {
    if (!self || !self->ctx || !config) {
        return inf_device_wifi_stub_impl_bad_argument_error("missing wifi stub ap config");
    }

    inf_device_wifi_stub_impl_ctx_t* ctx       = self->ctx;
    const char*                      ssid      = inf_device_wifi_stub_impl_bounded_strlen(config->ssid, sizeof(config->ssid)) > 0 ? config->ssid : inf_device_wifi_stub_impl_default_ssid(ctx);
    uint8_t                          channel   = config->channel > 0 ? config->channel : 1;
    dom_models_wifi_auth_mode_t      auth_mode = config->auth_mode == DOM_MODELS_WIFI_AUTH_UNKNOWN ? DOM_MODELS_WIFI_AUTH_WPA2_PSK : config->auth_mode;

    ctx->started    = true;
    ctx->ap_started = true;

    if (ctx->mode == DOM_MODELS_WIFI_MODE_STA || ctx->connected) {
        ctx->mode = DOM_MODELS_WIFI_MODE_APSTA;
    } else {
        ctx->mode = DOM_MODELS_WIFI_MODE_AP;
    }

    inf_device_wifi_stub_impl_fill_default_connected_ap(ctx, ssid, channel, auth_mode);

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t stop_ap_impl(
    dom_contracts_device_wifi_t* self
) {
    if (!self || !self->ctx) {
        return inf_device_wifi_stub_impl_bad_argument_error("missing wifi stub context");
    }

    inf_device_wifi_stub_impl_ctx_t* ctx = self->ctx;
    ctx->ap_started                     = false;

    if (ctx->mode == DOM_MODELS_WIFI_MODE_APSTA) {
        ctx->mode = DOM_MODELS_WIFI_MODE_STA;
    } else if (ctx->mode == DOM_MODELS_WIFI_MODE_AP) {
        ctx->mode    = DOM_MODELS_WIFI_MODE_NULL;
        ctx->started = false;
    }

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t start_scan_impl(
    dom_contracts_device_wifi_t*         self,
    const dom_models_wifi_scan_config_t* config
) {
    if (!self || !self->ctx) {
        return inf_device_wifi_stub_impl_bad_argument_error("missing wifi stub context");
    }

    inf_device_wifi_stub_impl_ctx_t* ctx     = self->ctx;
    const char*                      ssid    = inf_device_wifi_stub_impl_default_ssid(ctx);
    uint8_t                          channel = 1;

    if (config) {
        if (config->ssid_set && inf_device_wifi_stub_impl_bounded_strlen(config->ssid, sizeof(config->ssid)) > 0) {
            ssid = config->ssid;
        }
        if (config->channel_set) {
            channel = config->channel;
        }
    }

    inf_device_wifi_stub_impl_fill_default_scan(ctx, ssid, channel);
    ctx->scanned.scan_id += 1;

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t get_scanned_impl(
    dom_contracts_device_wifi_t*   self,
    dom_models_wifi_scan_result_t* out
) {
    if (!self || !self->ctx || !out) {
        return inf_device_wifi_stub_impl_bad_argument_error("missing wifi stub scan result argument");
    }

    inf_device_wifi_stub_impl_ctx_t* ctx = self->ctx;
    memcpy(out, &ctx->scanned, sizeof(dom_models_wifi_scan_result_t));

    return DOMAIN_MODELS_ERROR_OK;
}
