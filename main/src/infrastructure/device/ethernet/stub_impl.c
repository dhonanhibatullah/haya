#include "infrastructure/device/ethernet/stub_impl.h"

#include <stdlib.h>
#include <string.h>

#include "domain/contracts/device/ethernet.h"
#include "domain/models/error.h"
#include "domain/models/ethernet.h"

#define STUB_ETHERNET_IMPL_ACTOR "ethernet_stub"

/* Helper Function Prototypes */

static dom_models_error_t* bad_argument_error(const char* msg);
static size_t              bounded_strlen(const char* value, size_t max_len);
static void                copy_cstr(char* dst, size_t dst_size, const char* src);
static void                copy_if_key(bool* available, char* out, size_t out_size, const char* if_key);
static void                copy_mac(uint8_t dst[DOM_MODELS_ETHERNET_MAC_LEN], const uint8_t src[DOM_MODELS_ETHERNET_MAC_LEN]);
static bool                valid_fixed_link_config(const dom_models_ethernet_link_config_t* config);
static void                reset_runtime(inf_device_ethernet_stub_impl_ctx_t* ctx);

/* Contract Function Prototypes */

static dom_models_error_t* start_impl(
    dom_contracts_device_ethernet_t* self
);
static dom_models_error_t* stop_impl(
    dom_contracts_device_ethernet_t* self
);
static dom_models_error_t* get_capabilities_impl(
    dom_contracts_device_ethernet_t*    self,
    dom_models_ethernet_capabilities_t* out
);
static dom_models_error_t* get_status_impl(
    dom_contracts_device_ethernet_t* self,
    dom_models_ethernet_status_t*    out
);
static dom_models_error_t* set_mac_impl(
    dom_contracts_device_ethernet_t* self,
    const uint8_t                    mac[DOM_MODELS_ETHERNET_MAC_LEN]
);
static dom_models_error_t* set_link_config_impl(
    dom_contracts_device_ethernet_t*         self,
    const dom_models_ethernet_link_config_t* config
);
static dom_models_error_t* set_promiscuous_impl(
    dom_contracts_device_ethernet_t* self,
    bool                             enabled
);
static dom_models_error_t* set_flow_control_impl(
    dom_contracts_device_ethernet_t* self,
    bool                             enabled
);
static dom_models_error_t* set_phy_loopback_impl(
    dom_contracts_device_ethernet_t* self,
    bool                             enabled
);

/* Constructor and Destructor */

dom_contracts_device_ethernet_t* inf_device_ethernet_stub_impl_new(const inf_device_ethernet_stub_impl_cfg_t* cfg) {
    inf_device_ethernet_stub_impl_ctx_t* ctx = (inf_device_ethernet_stub_impl_ctx_t*)calloc(1, sizeof(inf_device_ethernet_stub_impl_ctx_t));
    if (!ctx) {
        return NULL;
    }

    if (!cfg) {
        inf_device_ethernet_stub_impl_cfg_t default_cfg = INF_DEVICE_ETHERNET_STUB_IMPL_CFG_DEFAULT();
        memcpy(&ctx->cfg, &default_cfg, sizeof(inf_device_ethernet_stub_impl_cfg_t));
    } else {
        memcpy(&ctx->cfg, cfg, sizeof(inf_device_ethernet_stub_impl_cfg_t));
    }

    copy_mac(ctx->mac, ctx->cfg.default_mac);
    ctx->link_config.autoneg = true;
    ctx->link_config.speed   = DOM_MODELS_ETHERNET_SPEED_100M;
    ctx->link_config.duplex  = DOM_MODELS_ETHERNET_DUPLEX_FULL;

    dom_contracts_device_ethernet_t* self = dom_contracts_device_ethernet_new(ctx);
    if (!self) {
        free(ctx);
        return NULL;
    }

    self->start            = start_impl;
    self->stop             = stop_impl;
    self->get_capabilities = get_capabilities_impl;
    self->get_status       = get_status_impl;
    self->set_mac          = set_mac_impl;
    self->set_link_config  = set_link_config_impl;
    self->set_promiscuous  = set_promiscuous_impl;
    self->set_flow_control = set_flow_control_impl;
    self->set_phy_loopback = set_phy_loopback_impl;

    return self;
}

void inf_device_ethernet_stub_impl_delete(dom_contracts_device_ethernet_t* self) {
    if (!self || !self->ctx) {
        return;
    }

    inf_device_ethernet_stub_impl_ctx_t* ctx = self->ctx;

    dom_models_error_t* err = inf_device_ethernet_stub_impl_deinit(self);
    dom_models_error_delete(err);

    free(ctx);
    dom_contracts_device_ethernet_delete(self);
}

dom_models_error_t* inf_device_ethernet_stub_impl_init(dom_contracts_device_ethernet_t* self) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing ethernet stub context");
    }

    inf_device_ethernet_stub_impl_ctx_t* ctx = self->ctx;
    ctx->initialized                         = true;

    return NULL;
}

dom_models_error_t* inf_device_ethernet_stub_impl_deinit(dom_contracts_device_ethernet_t* self) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing ethernet stub context");
    }

    inf_device_ethernet_stub_impl_ctx_t* ctx = self->ctx;

    reset_runtime(ctx);
    ctx->initialized = false;

    return NULL;
}

/* Contract Function Implementations */

static dom_models_error_t* start_impl(
    dom_contracts_device_ethernet_t* self
) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing ethernet stub context");
    }

    inf_device_ethernet_stub_impl_ctx_t* ctx = self->ctx;
    ctx->started                            = true;
    ctx->link_up                            = true;

    return NULL;
}

static dom_models_error_t* stop_impl(
    dom_contracts_device_ethernet_t* self
) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing ethernet stub context");
    }

    inf_device_ethernet_stub_impl_ctx_t* ctx = self->ctx;

    reset_runtime(ctx);

    return NULL;
}

static dom_models_error_t* get_capabilities_impl(
    dom_contracts_device_ethernet_t*    self,
    dom_models_ethernet_capabilities_t* out
) {
    if (!self || !self->ctx || !out) {
        return bad_argument_error("missing ethernet stub capabilities argument");
    }

    memset(out, 0, sizeof(dom_models_ethernet_capabilities_t));

    out->flags = DOM_MODELS_ETHERNET_CAPABILITY_SET_MAC |
                 DOM_MODELS_ETHERNET_CAPABILITY_LINK_CONFIG |
                 DOM_MODELS_ETHERNET_CAPABILITY_PROMISCUOUS |
                 DOM_MODELS_ETHERNET_CAPABILITY_FLOW_CONTROL |
                 DOM_MODELS_ETHERNET_CAPABILITY_PHY_LOOPBACK;

    return NULL;
}

static dom_models_error_t* get_status_impl(
    dom_contracts_device_ethernet_t* self,
    dom_models_ethernet_status_t*    out
) {
    if (!self || !self->ctx || !out) {
        return bad_argument_error("missing ethernet stub status argument");
    }

    inf_device_ethernet_stub_impl_ctx_t* ctx = self->ctx;

    memset(out, 0, sizeof(dom_models_ethernet_status_t));

    copy_if_key(&out->if_key_available, out->if_key, sizeof(out->if_key), ctx->cfg.if_key);

    out->started                = ctx->started;
    out->link_up                = ctx->link_up;
    out->mac_available          = true;
    out->phy_addr_available     = true;
    out->phy_addr               = ctx->cfg.phy_addr;
    out->autoneg_available      = true;
    out->autoneg                = ctx->link_config.autoneg;
    out->speed_available        = true;
    out->speed                  = ctx->link_config.speed;
    out->duplex_available       = true;
    out->duplex                 = ctx->link_config.duplex;
    out->promiscuous_available  = true;
    out->promiscuous            = ctx->promiscuous;
    out->flow_control_available = true;
    out->flow_control           = ctx->flow_control;
    out->phy_loopback_available = true;
    out->phy_loopback           = ctx->phy_loopback;

    copy_mac(out->mac, ctx->mac);

    return NULL;
}

static dom_models_error_t* set_mac_impl(
    dom_contracts_device_ethernet_t* self,
    const uint8_t                    mac[DOM_MODELS_ETHERNET_MAC_LEN]
) {
    if (!self || !self->ctx || !mac) {
        return bad_argument_error("missing ethernet stub mac argument");
    }

    inf_device_ethernet_stub_impl_ctx_t* ctx = self->ctx;

    copy_mac(ctx->mac, mac);

    return NULL;
}

static dom_models_error_t* set_link_config_impl(
    dom_contracts_device_ethernet_t*         self,
    const dom_models_ethernet_link_config_t* config
) {
    if (!self || !self->ctx || !config) {
        return bad_argument_error("missing ethernet stub link config");
    }

    if (!config->autoneg && !valid_fixed_link_config(config)) {
        return bad_argument_error("invalid ethernet stub fixed link config");
    }

    inf_device_ethernet_stub_impl_ctx_t* ctx = self->ctx;
    if (config->autoneg) {
        ctx->link_config.autoneg = true;
        ctx->link_config.speed   = DOM_MODELS_ETHERNET_SPEED_100M;
        ctx->link_config.duplex  = DOM_MODELS_ETHERNET_DUPLEX_FULL;
    } else {
        memcpy(&ctx->link_config, config, sizeof(dom_models_ethernet_link_config_t));
    }

    return NULL;
}

static dom_models_error_t* set_promiscuous_impl(
    dom_contracts_device_ethernet_t* self,
    bool                             enabled
) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing ethernet stub context");
    }

    inf_device_ethernet_stub_impl_ctx_t* ctx = self->ctx;
    ctx->promiscuous                       = enabled;

    return NULL;
}

static dom_models_error_t* set_flow_control_impl(
    dom_contracts_device_ethernet_t* self,
    bool                             enabled
) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing ethernet stub context");
    }

    inf_device_ethernet_stub_impl_ctx_t* ctx = self->ctx;
    ctx->flow_control                      = enabled;

    return NULL;
}

static dom_models_error_t* set_phy_loopback_impl(
    dom_contracts_device_ethernet_t* self,
    bool                             enabled
) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing ethernet stub context");
    }

    inf_device_ethernet_stub_impl_ctx_t* ctx = self->ctx;
    ctx->phy_loopback                     = enabled;

    return NULL;
}

/* Helper Function Implementations */

static dom_models_error_t* bad_argument_error(const char* msg) {
    return dom_models_error_new(STUB_ETHERNET_IMPL_ACTOR, DOMAIN_MODELS_ERROR_TYPE_BAD_ARGUMENT, "%s", msg ? msg : "bad argument");
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

static void copy_mac(uint8_t dst[DOM_MODELS_ETHERNET_MAC_LEN], const uint8_t src[DOM_MODELS_ETHERNET_MAC_LEN]) {
    if (!dst || !src) {
        return;
    }

    memcpy(dst, src, DOM_MODELS_ETHERNET_MAC_LEN);
}

static bool valid_fixed_link_config(const dom_models_ethernet_link_config_t* config) {
    if (!config) {
        return false;
    }

    return config->speed != DOM_MODELS_ETHERNET_SPEED_UNKNOWN &&
           config->duplex != DOM_MODELS_ETHERNET_DUPLEX_UNKNOWN;
}

static void reset_runtime(inf_device_ethernet_stub_impl_ctx_t* ctx) {
    if (!ctx) {
        return;
    }

    ctx->started      = false;
    ctx->link_up      = false;
    ctx->promiscuous  = false;
    ctx->flow_control = false;
    ctx->phy_loopback = false;

    copy_mac(ctx->mac, ctx->cfg.default_mac);
    ctx->link_config.autoneg = true;
    ctx->link_config.speed   = DOM_MODELS_ETHERNET_SPEED_100M;
    ctx->link_config.duplex  = DOM_MODELS_ETHERNET_DUPLEX_FULL;
}
