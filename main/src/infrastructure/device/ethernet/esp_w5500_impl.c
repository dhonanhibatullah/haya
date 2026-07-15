#include "infrastructure/device/ethernet/esp_w5500_impl.h"

#include <stdlib.h>
#include <string.h>

#include "domain/contracts/device/ethernet.h"
#include "domain/models/error.h"
#include "domain/models/ethernet.h"
#include "esp_err.h"
#include "esp_eth_driver.h"
#include "esp_eth_netif_glue.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_netif_defaults.h"
#include "infrastructure/device/ethernet/esp_w5500_impl_utils.h"

/* Event Handler Function Prototypes */

static void eth_event_handler(void* arg, esp_event_base_t base, int32_t id, void* data);
static void ip_event_handler(void* arg, esp_event_base_t base, int32_t id, void* data);
static void on_eth_start(inf_device_ethernet_esp_w5500_impl_ctx_t* ctx);
static void on_eth_stop(inf_device_ethernet_esp_w5500_impl_ctx_t* ctx);
static void on_eth_connected(inf_device_ethernet_esp_w5500_impl_ctx_t* ctx);
static void on_eth_disconnected(inf_device_ethernet_esp_w5500_impl_ctx_t* ctx);
static void on_eth_got_ip(inf_device_ethernet_esp_w5500_impl_ctx_t* ctx, const ip_event_got_ip_t* event);

/* Helper Function Prototypes */

static dom_models_error_t create_netif(inf_device_ethernet_esp_w5500_impl_ctx_t* ctx);
static void               dispatch_event(
    inf_device_ethernet_esp_w5500_impl_ctx_t* ctx,
    dom_models_ethernet_event_type_t          type,
    uint32_t                                  driver_status
);
static void clear_runtime(inf_device_ethernet_esp_w5500_impl_ctx_t* ctx);

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

dom_contracts_device_ethernet_t* inf_device_ethernet_esp_w5500_impl_new(const inf_device_ethernet_esp_w5500_impl_cfg_t* cfg) {
    inf_device_ethernet_esp_w5500_impl_ctx_t* ctx = (inf_device_ethernet_esp_w5500_impl_ctx_t*)calloc(1, sizeof(inf_device_ethernet_esp_w5500_impl_ctx_t));
    if (!ctx) {
        return NULL;
    }

    if (!cfg) {
        inf_device_ethernet_esp_w5500_impl_cfg_t default_cfg = INF_DEVICE_ETHERNET_ESP_W5500_IMPL_CFG_DEFAULT();
        memcpy(&ctx->cfg, &default_cfg, sizeof(inf_device_ethernet_esp_w5500_impl_cfg_t));
    } else {
        memcpy(&ctx->cfg, cfg, sizeof(inf_device_ethernet_esp_w5500_impl_cfg_t));
    }

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

void inf_device_ethernet_esp_w5500_impl_delete(dom_contracts_device_ethernet_t* self) {
    if (!self) {
        return;
    }

    inf_device_ethernet_esp_w5500_impl_ctx_t* ctx = self->ctx;
    if (ctx) {
        (void)inf_device_ethernet_esp_w5500_impl_deinit(self);
        free(ctx);
    }

    dom_contracts_device_ethernet_delete(self);
}

dom_models_error_t inf_device_ethernet_esp_w5500_impl_init(dom_contracts_device_ethernet_t* self) {
    if (!self || !self->ctx) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_device_ethernet_esp_w5500_impl_ctx_t* ctx = self->ctx;

    if (ctx->initialized) {
        return DOMAIN_MODELS_ERROR_OK;
    }

    if (!ctx->cfg.eth_handle) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    dom_models_error_t result = create_netif(ctx);
    if (result != DOMAIN_MODELS_ERROR_OK) {
        (void)inf_device_ethernet_esp_w5500_impl_deinit(self);
        return result;
    }

    if (ctx->cfg.attach_netif_glue) {
        ctx->netif_glue = esp_eth_new_netif_glue(ctx->cfg.eth_handle);
        if (!ctx->netif_glue) {
            (void)inf_device_ethernet_esp_w5500_impl_deinit(self);
            return DOMAIN_MODELS_ERROR_MALLOC_FAILED;
        }

        esp_err_t err = esp_netif_attach(ctx->netif, ctx->netif_glue);
        if (err != ESP_OK) {
            (void)inf_device_ethernet_esp_w5500_impl_deinit(self);
            return inf_device_ethernet_esp_w5500_impl_error_from_esp(err);
        }

        ctx->netif_attached = true;
    }

    if (ctx->cfg.register_event_handler) {
        esp_err_t err = esp_event_handler_instance_register(
            ETH_EVENT,
            ESP_EVENT_ANY_ID,
            eth_event_handler,
            ctx,
            &ctx->eth_event_handler
        );
        if (err != ESP_OK) {
            (void)inf_device_ethernet_esp_w5500_impl_deinit(self);
            return inf_device_ethernet_esp_w5500_impl_error_from_esp(err);
        }
        ctx->eth_event_handler_registered = true;
    }

    if (ctx->cfg.register_ip_event_handler) {
        esp_err_t err = esp_event_handler_instance_register(
            IP_EVENT,
            IP_EVENT_ETH_GOT_IP,
            ip_event_handler,
            ctx,
            &ctx->ip_event_handler
        );
        if (err != ESP_OK) {
            (void)inf_device_ethernet_esp_w5500_impl_deinit(self);
            return inf_device_ethernet_esp_w5500_impl_error_from_esp(err);
        }
        ctx->ip_event_handler_registered = true;
    }

    ctx->initialized = true;

    return DOMAIN_MODELS_ERROR_OK;
}

dom_models_error_t inf_device_ethernet_esp_w5500_impl_deinit(dom_contracts_device_ethernet_t* self) {
    if (!self || !self->ctx) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_device_ethernet_esp_w5500_impl_ctx_t* ctx    = self->ctx;
    dom_models_error_t                        result = DOMAIN_MODELS_ERROR_OK;

    if (ctx->started) {
        esp_err_t err = esp_eth_stop(ctx->cfg.eth_handle);
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE && result == DOMAIN_MODELS_ERROR_OK) {
            result = inf_device_ethernet_esp_w5500_impl_error_from_esp(err);
        }
    }

    if (ctx->ip_event_handler_registered) {
        esp_err_t err = esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_ETH_GOT_IP, ctx->ip_event_handler);
        if (err != ESP_OK) {
            if (result == DOMAIN_MODELS_ERROR_OK) {
                result = inf_device_ethernet_esp_w5500_impl_error_from_esp(err);
            }
        } else {
            ctx->ip_event_handler_registered = false;
            ctx->ip_event_handler            = NULL;
        }
    }

    if (ctx->eth_event_handler_registered) {
        esp_err_t err = esp_event_handler_instance_unregister(ETH_EVENT, ESP_EVENT_ANY_ID, ctx->eth_event_handler);
        if (err != ESP_OK) {
            if (result == DOMAIN_MODELS_ERROR_OK) {
                result = inf_device_ethernet_esp_w5500_impl_error_from_esp(err);
            }
        } else {
            ctx->eth_event_handler_registered = false;
            ctx->eth_event_handler            = NULL;
        }
    }

    if (ctx->netif_glue) {
        esp_err_t err = esp_eth_del_netif_glue(ctx->netif_glue);
        if (err != ESP_OK && result == DOMAIN_MODELS_ERROR_OK) {
            result = inf_device_ethernet_esp_w5500_impl_error_from_esp(err);
        }
        ctx->netif_glue     = NULL;
        ctx->netif_attached = false;
    }

    if (ctx->netif) {
        esp_netif_destroy(ctx->netif);
        ctx->netif = NULL;
    }

    ctx->initialized = false;
    clear_runtime(ctx);

    return result;
}

/* Contract Function Implementations */

static dom_models_error_t start_impl(
    dom_contracts_device_ethernet_t* self
) {
    if (!self || !self->ctx) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_device_ethernet_esp_w5500_impl_ctx_t* ctx = self->ctx;
    if (!ctx->initialized || !ctx->cfg.eth_handle) {
        return DOMAIN_MODELS_ERROR_BAD_STATE;
    }

    esp_err_t err = esp_eth_start(ctx->cfg.eth_handle);
    if (err != ESP_OK) {
        return inf_device_ethernet_esp_w5500_impl_error_from_esp(err);
    }

    ctx->started = true;

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t stop_impl(
    dom_contracts_device_ethernet_t* self
) {
    if (!self || !self->ctx) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_device_ethernet_esp_w5500_impl_ctx_t* ctx = self->ctx;
    if (!ctx->initialized || !ctx->cfg.eth_handle) {
        return DOMAIN_MODELS_ERROR_BAD_STATE;
    }

    esp_err_t err = esp_eth_stop(ctx->cfg.eth_handle);
    if (err != ESP_OK) {
        return inf_device_ethernet_esp_w5500_impl_error_from_esp(err);
    }

    ctx->started = false;
    ctx->link_up = false;

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
                 DOM_MODELS_ETHERNET_CAPABILITY_PROMISCUOUS;

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t get_status_impl(
    dom_contracts_device_ethernet_t* self,
    dom_models_ethernet_status_t*    out
) {
    if (!self || !self->ctx || !out) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_device_ethernet_esp_w5500_impl_ctx_t* ctx = self->ctx;

    memset(out, 0, sizeof(dom_models_ethernet_status_t));

    inf_device_ethernet_esp_w5500_impl_copy_if_key(&out->if_key_available, out->if_key, sizeof(out->if_key), ctx->cfg.if_key);

    out->started = ctx->started;
    out->link_up = ctx->link_up;

    if (!ctx->cfg.eth_handle) {
        return DOMAIN_MODELS_ERROR_OK;
    }

    esp_err_t err = esp_eth_ioctl(ctx->cfg.eth_handle, ETH_CMD_G_MAC_ADDR, out->mac);
    if (err == ESP_OK) {
        out->mac_available = true;
    }

    uint32_t phy_addr = 0;
    err               = esp_eth_ioctl(ctx->cfg.eth_handle, ETH_CMD_G_PHY_ADDR, &phy_addr);
    if (err == ESP_OK) {
        out->phy_addr_available = true;
        out->phy_addr           = phy_addr;
    }

    bool autoneg = false;
    err          = esp_eth_ioctl(ctx->cfg.eth_handle, ETH_CMD_G_AUTONEGO, &autoneg);
    if (err == ESP_OK) {
        out->autoneg_available = true;
        out->autoneg           = autoneg;
    }

    eth_speed_t speed = ETH_SPEED_10M;
    err               = esp_eth_ioctl(ctx->cfg.eth_handle, ETH_CMD_G_SPEED, &speed);
    if (err == ESP_OK) {
        out->speed_available = true;
        out->speed           = inf_device_ethernet_esp_w5500_impl_speed_to_domain(speed);
    }

    eth_duplex_t duplex = ETH_DUPLEX_HALF;
    err                 = esp_eth_ioctl(ctx->cfg.eth_handle, ETH_CMD_G_DUPLEX_MODE, &duplex);
    if (err == ESP_OK) {
        out->duplex_available = true;
        out->duplex           = inf_device_ethernet_esp_w5500_impl_duplex_to_domain(duplex);
    }

    out->promiscuous_available = true;
    out->promiscuous           = ctx->promiscuous;

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t set_mac_impl(
    dom_contracts_device_ethernet_t* self,
    const uint8_t                    mac[DOM_MODELS_ETHERNET_MAC_LEN]
) {
    if (!self || !self->ctx || !mac) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_device_ethernet_esp_w5500_impl_ctx_t* ctx = self->ctx;
    if (!ctx->initialized || !ctx->cfg.eth_handle) {
        return DOMAIN_MODELS_ERROR_BAD_STATE;
    }

    esp_err_t err = esp_eth_ioctl(ctx->cfg.eth_handle, ETH_CMD_S_MAC_ADDR, (void*)mac);
    if (err != ESP_OK) {
        return inf_device_ethernet_esp_w5500_impl_error_from_esp(err);
    }

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t set_link_config_impl(
    dom_contracts_device_ethernet_t*         self,
    const dom_models_ethernet_link_config_t* config
) {
    if (!self || !self->ctx || !config) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_device_ethernet_esp_w5500_impl_ctx_t* ctx = self->ctx;
    if (!ctx->initialized || !ctx->cfg.eth_handle) {
        return DOMAIN_MODELS_ERROR_BAD_STATE;
    }

    eth_speed_t  speed;
    eth_duplex_t duplex;
    if (!config->autoneg &&
        (!inf_device_ethernet_esp_w5500_impl_speed_from_domain(config->speed, &speed) ||
         !inf_device_ethernet_esp_w5500_impl_duplex_from_domain(config->duplex, &duplex))) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    bool      autoneg = config->autoneg;
    esp_err_t err     = esp_eth_ioctl(ctx->cfg.eth_handle, ETH_CMD_S_AUTONEGO, &autoneg);
    if (err != ESP_OK) {
        return inf_device_ethernet_esp_w5500_impl_error_from_esp(err);
    }

    if (config->autoneg) {
        return DOMAIN_MODELS_ERROR_OK;
    }

    err = esp_eth_ioctl(ctx->cfg.eth_handle, ETH_CMD_S_SPEED, &speed);
    if (err != ESP_OK) {
        return inf_device_ethernet_esp_w5500_impl_error_from_esp(err);
    }

    err = esp_eth_ioctl(ctx->cfg.eth_handle, ETH_CMD_S_DUPLEX_MODE, &duplex);
    if (err != ESP_OK) {
        return inf_device_ethernet_esp_w5500_impl_error_from_esp(err);
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

    inf_device_ethernet_esp_w5500_impl_ctx_t* ctx = self->ctx;
    if (!ctx->initialized || !ctx->cfg.eth_handle) {
        return DOMAIN_MODELS_ERROR_BAD_STATE;
    }

    esp_err_t err = esp_eth_ioctl(ctx->cfg.eth_handle, ETH_CMD_S_PROMISCUOUS, &enabled);
    if (err != ESP_OK) {
        return inf_device_ethernet_esp_w5500_impl_error_from_esp(err);
    }

    ctx->promiscuous = enabled;

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t set_flow_control_impl(
    dom_contracts_device_ethernet_t* self,
    bool                             enabled
) {
    if (!self || !self->ctx) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    (void)enabled;

    return DOMAIN_MODELS_ERROR_NOT_SUPPORTED;
}

static dom_models_error_t set_phy_loopback_impl(
    dom_contracts_device_ethernet_t* self,
    bool                             enabled
) {
    if (!self || !self->ctx) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    (void)enabled;

    return DOMAIN_MODELS_ERROR_NOT_SUPPORTED;
}

static dom_models_error_t add_event_callback_impl(
    dom_contracts_device_ethernet_t*     self,
    void*                                cb_ctx,
    dom_models_ethernet_event_callback_t cb_func
) {
    if (!self || !self->ctx || !cb_func) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_device_ethernet_esp_w5500_impl_ctx_t* ctx = self->ctx;

    for (size_t i = 0; i < ctx->event_cb_cnt; i++) {
        if (ctx->event_cb_funcs[i] == cb_func) {
            return DOMAIN_MODELS_ERROR_OK;
        }
    }

    if (ctx->event_cb_cnt >= INF_DEVICE_ETHERNET_ESP_W5500_IMPL_EVENT_CALLBACK_MAX) {
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

    inf_device_ethernet_esp_w5500_impl_ctx_t* ctx = self->ctx;

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

static void eth_event_handler(void* arg, esp_event_base_t base, int32_t id, void* data) {
    if (!arg || base != ETH_EVENT) {
        return;
    }

    (void)data;

    inf_device_ethernet_esp_w5500_impl_ctx_t* ctx = arg;

    switch (id) {
        case ETHERNET_EVENT_START:
            on_eth_start(ctx);
            break;
        case ETHERNET_EVENT_STOP:
            on_eth_stop(ctx);
            break;
        case ETHERNET_EVENT_CONNECTED:
            on_eth_connected(ctx);
            break;
        case ETHERNET_EVENT_DISCONNECTED:
            on_eth_disconnected(ctx);
            break;
        default:
            break;
    }
}

static void ip_event_handler(void* arg, esp_event_base_t base, int32_t id, void* data) {
    if (!arg || base != IP_EVENT || id != IP_EVENT_ETH_GOT_IP) {
        return;
    }

    on_eth_got_ip(arg, data);
}

static void on_eth_start(inf_device_ethernet_esp_w5500_impl_ctx_t* ctx) {
    if (!ctx) {
        return;
    }

    ctx->started = true;
    dispatch_event(ctx, DOM_MODELS_ETHERNET_EVENT_STARTED, 0);
}

static void on_eth_stop(inf_device_ethernet_esp_w5500_impl_ctx_t* ctx) {
    if (!ctx) {
        return;
    }

    ctx->started = false;
    ctx->link_up = false;
    dispatch_event(ctx, DOM_MODELS_ETHERNET_EVENT_STOPPED, 0);
}

static void on_eth_connected(inf_device_ethernet_esp_w5500_impl_ctx_t* ctx) {
    if (!ctx) {
        return;
    }

    ctx->link_up = true;
    dispatch_event(ctx, DOM_MODELS_ETHERNET_EVENT_LINK_UP, 0);
}

static void on_eth_disconnected(inf_device_ethernet_esp_w5500_impl_ctx_t* ctx) {
    if (!ctx) {
        return;
    }

    ctx->link_up = false;
    dispatch_event(ctx, DOM_MODELS_ETHERNET_EVENT_LINK_DOWN, 0);
}

static void on_eth_got_ip(inf_device_ethernet_esp_w5500_impl_ctx_t* ctx, const ip_event_got_ip_t* event) {
    if (!ctx) {
        return;
    }

    dispatch_event(ctx, DOM_MODELS_ETHERNET_EVENT_GOT_IP, event ? (uint32_t)event->ip_changed : 0);
}

/* Helper Function Implementations */

static dom_models_error_t create_netif(inf_device_ethernet_esp_w5500_impl_ctx_t* ctx) {
    if (!ctx) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    esp_netif_inherent_config_t base_cfg;
    memcpy(&base_cfg, ESP_NETIF_BASE_DEFAULT_ETH, sizeof(esp_netif_inherent_config_t));
    if (ctx->cfg.if_key) {
        base_cfg.if_key = ctx->cfg.if_key;
    }
    if (ctx->cfg.if_desc) {
        base_cfg.if_desc = ctx->cfg.if_desc;
    }
    base_cfg.route_prio = ctx->cfg.route_prio;

    esp_netif_config_t netif_cfg = ESP_NETIF_DEFAULT_ETH();
    netif_cfg.base               = &base_cfg;

    ctx->netif = esp_netif_new(&netif_cfg);
    if (!ctx->netif) {
        return DOMAIN_MODELS_ERROR_MALLOC_FAILED;
    }

    return DOMAIN_MODELS_ERROR_OK;
}

static void dispatch_event(
    inf_device_ethernet_esp_w5500_impl_ctx_t* ctx,
    dom_models_ethernet_event_type_t          type,
    uint32_t                                  driver_status
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

static void clear_runtime(inf_device_ethernet_esp_w5500_impl_ctx_t* ctx) {
    if (!ctx) {
        return;
    }

    ctx->started     = false;
    ctx->link_up     = false;
    ctx->promiscuous = false;
}
