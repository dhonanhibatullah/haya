#include "infrastructure/device/ethernet/stub_impl.h"

#include <stdlib.h>
#include <string.h>

#include "domain/contracts/device/ethernet.h"
#include "domain/models/error.h"
#include "domain/models/ethernet.h"
#include "infrastructure/device/ethernet/stub_impl_utils.h"

/* Helper Function Prototypes */

static void dispatch_event(
    inf_device_ethernet_stub_impl_ctx_t* ctx,
    dom_models_ethernet_event_type_t     type,
    uint32_t                             driver_status
);

static void reset_runtime(inf_device_ethernet_stub_impl_ctx_t* ctx);

/* Contract Function Prototypes */

static dom_models_error_t start_impl(
    dom_contracts_device_ethernet_t* self
);
static dom_models_error_t stop_impl(
    dom_contracts_device_ethernet_t* self
);
static dom_models_error_t get_capabilities_impl(
    dom_contracts_device_ethernet_t*    self,
    dom_models_ethernet_capabilities_t* out
);
static dom_models_error_t get_status_impl(
    dom_contracts_device_ethernet_t* self,
    dom_models_ethernet_status_t*    out
);
static dom_models_error_t set_mac_impl(
    dom_contracts_device_ethernet_t* self,
    const uint8_t                    mac[DOM_MODELS_ETHERNET_MAC_LEN]
);
static dom_models_error_t set_link_config_impl(
    dom_contracts_device_ethernet_t*         self,
    const dom_models_ethernet_link_config_t* config
);
static dom_models_error_t set_promiscuous_impl(
    dom_contracts_device_ethernet_t* self,
    bool                             enabled
);
static dom_models_error_t set_flow_control_impl(
    dom_contracts_device_ethernet_t* self,
    bool                             enabled
);
static dom_models_error_t set_phy_loopback_impl(
    dom_contracts_device_ethernet_t* self,
    bool                             enabled
);
static dom_models_error_t add_event_callback_impl(
    dom_contracts_device_ethernet_t*     self,
    void*                                cb_ctx,
    dom_models_ethernet_event_callback_t cb_func
);
static dom_models_error_t remove_event_callback_impl(
    dom_contracts_device_ethernet_t*     self,
    dom_models_ethernet_event_callback_t cb_func
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

    inf_device_ethernet_stub_impl_copy_mac(ctx->mac, ctx->cfg.default_mac);
    ctx->link_config.autoneg = true;
    ctx->link_config.speed   = DOM_MODELS_ETHERNET_SPEED_100M;
    ctx->link_config.duplex  = DOM_MODELS_ETHERNET_DUPLEX_FULL;

    dom_contracts_device_ethernet_t* self = dom_contracts_device_ethernet_new(ctx);
    if (!self) {
        free(ctx);
        return NULL;
    }

    self->start                 = start_impl;
    self->stop                  = stop_impl;
    self->get_capabilities      = get_capabilities_impl;
    self->get_status            = get_status_impl;
    self->set_mac               = set_mac_impl;
    self->set_link_config       = set_link_config_impl;
    self->set_promiscuous       = set_promiscuous_impl;
    self->set_flow_control      = set_flow_control_impl;
    self->set_phy_loopback      = set_phy_loopback_impl;
    self->add_event_callback    = add_event_callback_impl;
    self->remove_event_callback = remove_event_callback_impl;

    return self;
}

void inf_device_ethernet_stub_impl_delete(dom_contracts_device_ethernet_t* self) {
    if (!self) {
        return;
    }

    free(self->ctx);
    dom_contracts_device_ethernet_delete(self);
}

dom_models_error_t inf_device_ethernet_stub_impl_init(dom_contracts_device_ethernet_t* self) {
    if (!self || !self->ctx) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_device_ethernet_stub_impl_ctx_t* ctx = self->ctx;
    ctx->initialized                         = true;

    return DOMAIN_MODELS_ERROR_OK;
}

dom_models_error_t inf_device_ethernet_stub_impl_deinit(dom_contracts_device_ethernet_t* self) {
    if (!self || !self->ctx) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_device_ethernet_stub_impl_ctx_t* ctx = self->ctx;

    reset_runtime(ctx);
    ctx->initialized = false;

    return DOMAIN_MODELS_ERROR_OK;
}

/* Contract Function Implementations */

static dom_models_error_t start_impl(
    dom_contracts_device_ethernet_t* self
) {
    if (!self || !self->ctx) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_device_ethernet_stub_impl_ctx_t* ctx = self->ctx;
    if (!ctx->initialized) {
        return DOMAIN_MODELS_ERROR_BAD_STATE;
    }

    ctx->started = true;
    ctx->link_up = true;

    dispatch_event(ctx, DOM_MODELS_ETHERNET_EVENT_STARTED, 0);
    dispatch_event(ctx, DOM_MODELS_ETHERNET_EVENT_LINK_UP, 0);

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t stop_impl(
    dom_contracts_device_ethernet_t* self
) {
    if (!self || !self->ctx) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_device_ethernet_stub_impl_ctx_t* ctx = self->ctx;

    if (ctx->link_up) {
        dispatch_event(ctx, DOM_MODELS_ETHERNET_EVENT_LINK_DOWN, 0);
    }
    if (ctx->started) {
        dispatch_event(ctx, DOM_MODELS_ETHERNET_EVENT_STOPPED, 0);
    }

    reset_runtime(ctx);

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t get_capabilities_impl(
    dom_contracts_device_ethernet_t*    self,
    dom_models_ethernet_capabilities_t* out
) {
    if (!self || !self->ctx || !out) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    memset(out, 0, sizeof(dom_models_ethernet_capabilities_t));

    out->flags = DOM_MODELS_ETHERNET_CAPABILITY_SET_MAC |
                 DOM_MODELS_ETHERNET_CAPABILITY_LINK_CONFIG |
                 DOM_MODELS_ETHERNET_CAPABILITY_PROMISCUOUS |
                 DOM_MODELS_ETHERNET_CAPABILITY_FLOW_CONTROL |
                 DOM_MODELS_ETHERNET_CAPABILITY_PHY_LOOPBACK;

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t get_status_impl(
    dom_contracts_device_ethernet_t* self,
    dom_models_ethernet_status_t*    out
) {
    if (!self || !self->ctx || !out) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_device_ethernet_stub_impl_ctx_t* ctx = self->ctx;

    memset(out, 0, sizeof(dom_models_ethernet_status_t));

    inf_device_ethernet_stub_impl_copy_if_key(&out->if_key_available, out->if_key, sizeof(out->if_key), ctx->cfg.if_key);

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

    inf_device_ethernet_stub_impl_copy_mac(out->mac, ctx->mac);

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t set_mac_impl(
    dom_contracts_device_ethernet_t* self,
    const uint8_t                    mac[DOM_MODELS_ETHERNET_MAC_LEN]
) {
    if (!self || !self->ctx || !mac) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_device_ethernet_stub_impl_ctx_t* ctx = self->ctx;
    inf_device_ethernet_stub_impl_copy_mac(ctx->mac, mac);

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t set_link_config_impl(
    dom_contracts_device_ethernet_t*         self,
    const dom_models_ethernet_link_config_t* config
) {
    if (!self || !self->ctx || !config) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    if (!inf_device_ethernet_stub_impl_valid_fixed_link_config(config)) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_device_ethernet_stub_impl_ctx_t* ctx = self->ctx;
    if (config->autoneg) {
        ctx->link_config.autoneg = true;
        ctx->link_config.speed   = DOM_MODELS_ETHERNET_SPEED_100M;
        ctx->link_config.duplex  = DOM_MODELS_ETHERNET_DUPLEX_FULL;
    } else {
        memcpy(&ctx->link_config, config, sizeof(dom_models_ethernet_link_config_t));
    }

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t set_promiscuous_impl(
    dom_contracts_device_ethernet_t* self,
    bool                             enabled
) {
    if (!self || !self->ctx) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_device_ethernet_stub_impl_ctx_t* ctx = self->ctx;
    ctx->promiscuous                         = enabled;

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t set_flow_control_impl(
    dom_contracts_device_ethernet_t* self,
    bool                             enabled
) {
    if (!self || !self->ctx) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_device_ethernet_stub_impl_ctx_t* ctx = self->ctx;
    ctx->flow_control                        = enabled;

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t set_phy_loopback_impl(
    dom_contracts_device_ethernet_t* self,
    bool                             enabled
) {
    if (!self || !self->ctx) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_device_ethernet_stub_impl_ctx_t* ctx = self->ctx;
    ctx->phy_loopback                        = enabled;

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t add_event_callback_impl(
    dom_contracts_device_ethernet_t*     self,
    void*                                cb_ctx,
    dom_models_ethernet_event_callback_t cb_func
) {
    if (!self || !self->ctx || !cb_func) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_device_ethernet_stub_impl_ctx_t* ctx = self->ctx;

    for (size_t i = 0; i < ctx->event_cb_cnt; i++) {
        if (ctx->event_cb_funcs[i] == cb_func) {
            return DOMAIN_MODELS_ERROR_OK;
        }
    }

    if (ctx->event_cb_cnt >= INF_DEVICE_ETHERNET_STUB_IMPL_EVENT_CALLBACK_MAX) {
        return DOMAIN_MODELS_ERROR_BAD_STATE;
    }

    ctx->event_cb_funcs[ctx->event_cb_cnt] = cb_func;
    ctx->event_cb_ctxs[ctx->event_cb_cnt]  = cb_ctx;
    ctx->event_cb_cnt += 1;

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t remove_event_callback_impl(
    dom_contracts_device_ethernet_t*     self,
    dom_models_ethernet_event_callback_t cb_func
) {
    if (!self || !self->ctx || !cb_func) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_device_ethernet_stub_impl_ctx_t* ctx = self->ctx;

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

/* Helper Function Implementations */

static void dispatch_event(
    inf_device_ethernet_stub_impl_ctx_t* ctx,
    dom_models_ethernet_event_type_t     type,
    uint32_t                             driver_status
) {
    if (!ctx) {
        return;
    }

    dom_models_ethernet_event_t event = {
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

static void reset_runtime(inf_device_ethernet_stub_impl_ctx_t* ctx) {
    if (!ctx) {
        return;
    }

    ctx->started      = false;
    ctx->link_up      = false;
    ctx->promiscuous  = false;
    ctx->flow_control = false;
    ctx->phy_loopback = false;
}
