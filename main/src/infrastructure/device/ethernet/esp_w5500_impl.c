#include "infrastructure/device/ethernet/esp_w5500_impl.h"

#include <stdlib.h>
#include <string.h>

#include "domain/contracts/device/ethernet.h"
#include "domain/models/error.h"
#include "domain/models/ethernet.h"
#include "esp_err.h"
#include "esp_eth_driver.h"
#include "esp_eth_mac.h"
#include "esp_eth_mac_w5500.h"
#include "esp_eth_phy.h"
#include "esp_eth_phy_w5500.h"
#include "esp_event.h"

#define ESP_W5500_IMPL_ACTOR "esp_w5500"

/* Helper Function Prototypes */

static dom_models_error_t*          bad_argument_error(const char* msg);
static dom_models_error_t*          not_supported_error(const char* msg);
static dom_models_error_t*          error_from_esp(const char* action, esp_err_t err);
static dom_models_error_t*          append_error_from_esp(dom_models_error_t* handle, const char* action, esp_err_t err);
static size_t                       bounded_strlen(const char* value, size_t max_len);
static void                         copy_cstr(char* dst, size_t dst_size, const char* src);
static void                         copy_if_key(bool* available, char* out, size_t out_size, const char* if_key);
static bool                         validate_config(const inf_device_ethernet_esp_w5500_impl_cfg_t* cfg);
static bool                         speed_from_domain(dom_models_ethernet_speed_t in, eth_speed_t* out);
static dom_models_ethernet_speed_t  speed_to_domain(eth_speed_t in);
static bool                         duplex_from_domain(dom_models_ethernet_duplex_t in, eth_duplex_t* out);
static dom_models_ethernet_duplex_t duplex_to_domain(eth_duplex_t in);
static void                         clear_runtime(inf_device_ethernet_esp_w5500_impl_ctx_t* ctx);

/* Event Handler Function Prototypes */

static void eth_event_handler(void* arg, esp_event_base_t base, int32_t id, void* data);
static void on_eth_start(inf_device_ethernet_esp_w5500_impl_ctx_t* ctx);
static void on_eth_stop(inf_device_ethernet_esp_w5500_impl_ctx_t* ctx);
static void on_eth_connected(inf_device_ethernet_esp_w5500_impl_ctx_t* ctx);
static void on_eth_disconnected(inf_device_ethernet_esp_w5500_impl_ctx_t* ctx);

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

void inf_device_ethernet_esp_w5500_impl_delete(dom_contracts_device_ethernet_t* self) {
    if (!self || !self->ctx) {
        return;
    }

    inf_device_ethernet_esp_w5500_impl_ctx_t* ctx = self->ctx;

    dom_models_error_t* err = inf_device_ethernet_esp_w5500_impl_deinit(self);
    dom_models_error_delete(err);

    free(ctx);
    dom_contracts_device_ethernet_delete(self);
}

dom_models_error_t* inf_device_ethernet_esp_w5500_impl_init(dom_contracts_device_ethernet_t* self) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing ethernet context");
    }

    inf_device_ethernet_esp_w5500_impl_ctx_t* ctx = self->ctx;

    if (ctx->eth_initialized) {
        return NULL;
    }

    if (!validate_config(&ctx->cfg)) {
        return bad_argument_error("invalid w5500 config");
    }

    spi_device_interface_config_t spi_device_config;
    memset(&spi_device_config, 0, sizeof(spi_device_interface_config_t));
    spi_device_config.mode           = ctx->cfg.spi_mode;
    spi_device_config.clock_speed_hz = ctx->cfg.spi_clock_hz;
    spi_device_config.spics_io_num   = ctx->cfg.spi_cs_gpio_num;
    spi_device_config.queue_size     = ctx->cfg.spi_queue_size;

    eth_w5500_config_t w5500_config  = ETH_W5500_DEFAULT_CONFIG(ctx->cfg.spi_host, &spi_device_config);
    w5500_config.base.int_gpio_num   = ctx->cfg.int_gpio_num;
    w5500_config.base.poll_period_ms = ctx->cfg.poll_period_ms;

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.reset_gpio_num   = ctx->cfg.reset_gpio_num;

    ctx->mac = esp_eth_mac_new_w5500(&w5500_config, &mac_config);
    if (!ctx->mac) {
        return error_from_esp("esp_eth_mac_new_w5500", ESP_FAIL);
    }

    ctx->phy = esp_eth_phy_new_w5500(&phy_config);
    if (!ctx->phy) {
        ctx->mac->del(ctx->mac);
        ctx->mac = NULL;
        return error_from_esp("esp_eth_phy_new_w5500", ESP_FAIL);
    }

    esp_eth_config_t eth_config = ETH_DEFAULT_CONFIG(ctx->mac, ctx->phy);

    esp_err_t err = esp_eth_driver_install(&eth_config, &ctx->eth_handle);
    if (err != ESP_OK) {
        ctx->phy->del(ctx->phy);
        ctx->mac->del(ctx->mac);
        ctx->phy = NULL;
        ctx->mac = NULL;
        return error_from_esp("esp_eth_driver_install", err);
    }
    ctx->eth_initialized = true;

    if (ctx->cfg.register_event_handler) {
        err = esp_event_handler_instance_register(
            ETH_EVENT,
            ESP_EVENT_ANY_ID,
            eth_event_handler,
            ctx,
            &ctx->eth_event_handler
        );
        if (err != ESP_OK) {
            dom_models_error_t* err_handle = error_from_esp("esp_event_handler_instance_register", err);

            err = esp_eth_driver_uninstall(ctx->eth_handle);
            if (err != ESP_OK) {
                err_handle = append_error_from_esp(err_handle, "esp_eth_driver_uninstall", err);
                return err_handle;
            }

            if (ctx->phy) {
                err = ctx->phy->del(ctx->phy);
                if (err != ESP_OK) {
                    err_handle = append_error_from_esp(err_handle, "phy->del", err);
                }
            }
            if (ctx->mac) {
                err = ctx->mac->del(ctx->mac);
                if (err != ESP_OK) {
                    err_handle = append_error_from_esp(err_handle, "mac->del", err);
                }
            }

            ctx->eth_handle      = NULL;
            ctx->phy             = NULL;
            ctx->mac             = NULL;
            ctx->eth_initialized = false;
            clear_runtime(ctx);

            return err_handle;
        }
        ctx->eth_event_handler_registered = true;
    }

    return NULL;
}

dom_models_error_t* inf_device_ethernet_esp_w5500_impl_deinit(dom_contracts_device_ethernet_t* self) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing ethernet context");
    }

    inf_device_ethernet_esp_w5500_impl_ctx_t* ctx        = self->ctx;
    dom_models_error_t*                       err_handle = NULL;

    if (ctx->started && ctx->eth_handle) {
        esp_err_t err = esp_eth_stop(ctx->eth_handle);
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
            err_handle = append_error_from_esp(err_handle, "esp_eth_stop", err);
        }
    }

    if (ctx->eth_event_handler_registered) {
        esp_err_t err = esp_event_handler_instance_unregister(ETH_EVENT, ESP_EVENT_ANY_ID, ctx->eth_event_handler);
        if (err != ESP_OK) {
            err_handle = append_error_from_esp(err_handle, "esp_event_handler_instance_unregister", err);
        } else {
            ctx->eth_event_handler_registered = false;
            ctx->eth_event_handler            = NULL;
        }
    }

    if (ctx->eth_handle) {
        esp_err_t err = esp_eth_driver_uninstall(ctx->eth_handle);
        if (err != ESP_OK) {
            err_handle = append_error_from_esp(err_handle, "esp_eth_driver_uninstall", err);
        } else {
            ctx->eth_handle      = NULL;
            ctx->eth_initialized = false;
        }
    }

    if (!ctx->eth_handle && ctx->phy) {
        esp_err_t err = ctx->phy->del(ctx->phy);
        if (err != ESP_OK) {
            err_handle = append_error_from_esp(err_handle, "phy->del", err);
        } else {
            ctx->phy = NULL;
        }
    }

    if (!ctx->eth_handle && ctx->mac) {
        esp_err_t err = ctx->mac->del(ctx->mac);
        if (err != ESP_OK) {
            err_handle = append_error_from_esp(err_handle, "mac->del", err);
        } else {
            ctx->mac = NULL;
        }
    }

    if (!ctx->eth_handle) {
        clear_runtime(ctx);
    }

    return err_handle;
}

esp_eth_handle_t inf_device_ethernet_esp_w5500_impl_get_handle(dom_contracts_device_ethernet_t* self) {
    if (!self || !self->ctx) {
        return NULL;
    }

    inf_device_ethernet_esp_w5500_impl_ctx_t* ctx = self->ctx;

    return ctx->eth_handle;
}

/* Contract Function Implementations */

static dom_models_error_t* start_impl(
    dom_contracts_device_ethernet_t* self
) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing ethernet context");
    }

    inf_device_ethernet_esp_w5500_impl_ctx_t* ctx = self->ctx;
    if (!ctx->eth_handle) {
        return bad_argument_error("ethernet is not initialized");
    }

    esp_err_t err = esp_eth_start(ctx->eth_handle);
    if (err != ESP_OK) {
        return error_from_esp("esp_eth_start", err);
    }

    ctx->started = true;

    return NULL;
}

static dom_models_error_t* stop_impl(
    dom_contracts_device_ethernet_t* self
) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing ethernet context");
    }

    inf_device_ethernet_esp_w5500_impl_ctx_t* ctx = self->ctx;
    if (!ctx->eth_handle) {
        return bad_argument_error("ethernet is not initialized");
    }

    esp_err_t err = esp_eth_stop(ctx->eth_handle);
    if (err != ESP_OK) {
        return error_from_esp("esp_eth_stop", err);
    }

    ctx->started = false;
    ctx->link_up = false;

    return NULL;
}

static dom_models_error_t* get_capabilities_impl(
    dom_contracts_device_ethernet_t*    self,
    dom_models_ethernet_capabilities_t* out
) {
    if (!self || !self->ctx || !out) {
        return bad_argument_error("missing ethernet capabilities argument");
    }

    memset(out, 0, sizeof(dom_models_ethernet_capabilities_t));
    out->flags = DOM_MODELS_ETHERNET_CAPABILITY_SET_MAC |
                 DOM_MODELS_ETHERNET_CAPABILITY_LINK_CONFIG |
                 DOM_MODELS_ETHERNET_CAPABILITY_PROMISCUOUS;

    return NULL;
}

static dom_models_error_t* get_status_impl(
    dom_contracts_device_ethernet_t* self,
    dom_models_ethernet_status_t*    out
) {
    if (!self || !self->ctx || !out) {
        return bad_argument_error("missing ethernet status argument");
    }

    inf_device_ethernet_esp_w5500_impl_ctx_t* ctx = self->ctx;

    memset(out, 0, sizeof(dom_models_ethernet_status_t));

    copy_if_key(&out->if_key_available, out->if_key, sizeof(out->if_key), ctx->cfg.if_key);

    out->started = ctx->started;
    out->link_up = ctx->link_up;

    if (!ctx->eth_handle) {
        return NULL;
    }

    esp_err_t err = esp_eth_ioctl(ctx->eth_handle, ETH_CMD_G_MAC_ADDR, out->mac);
    if (err == ESP_OK) {
        out->mac_available = true;
    }

    uint32_t phy_addr = 0;
    err               = esp_eth_ioctl(ctx->eth_handle, ETH_CMD_G_PHY_ADDR, &phy_addr);
    if (err == ESP_OK) {
        out->phy_addr_available = true;
        out->phy_addr           = phy_addr;
    }

    bool autoneg = false;
    err          = esp_eth_ioctl(ctx->eth_handle, ETH_CMD_G_AUTONEGO, &autoneg);
    if (err == ESP_OK) {
        out->autoneg_available = true;
        out->autoneg           = autoneg;
    }

    eth_speed_t speed = ETH_SPEED_10M;
    err               = esp_eth_ioctl(ctx->eth_handle, ETH_CMD_G_SPEED, &speed);
    if (err == ESP_OK) {
        out->speed_available = true;
        out->speed           = speed_to_domain(speed);
    }

    eth_duplex_t duplex = ETH_DUPLEX_HALF;
    err                 = esp_eth_ioctl(ctx->eth_handle, ETH_CMD_G_DUPLEX_MODE, &duplex);
    if (err == ESP_OK) {
        out->duplex_available = true;
        out->duplex           = duplex_to_domain(duplex);
    }

    out->promiscuous_available = true;
    out->promiscuous           = ctx->promiscuous;
    out->flow_control          = ctx->flow_control;
    out->phy_loopback          = ctx->phy_loopback;

    return NULL;
}

static dom_models_error_t* set_mac_impl(
    dom_contracts_device_ethernet_t* self,
    const uint8_t                    mac[DOM_MODELS_ETHERNET_MAC_LEN]
) {
    if (!self || !self->ctx || !mac) {
        return bad_argument_error("missing ethernet mac argument");
    }

    inf_device_ethernet_esp_w5500_impl_ctx_t* ctx = self->ctx;
    if (!ctx->eth_handle) {
        return bad_argument_error("ethernet is not initialized");
    }

    esp_err_t err = esp_eth_ioctl(ctx->eth_handle, ETH_CMD_S_MAC_ADDR, (void*)mac);
    if (err != ESP_OK) {
        return error_from_esp("esp_eth_ioctl ETH_CMD_S_MAC_ADDR", err);
    }

    return NULL;
}

static dom_models_error_t* set_link_config_impl(
    dom_contracts_device_ethernet_t*         self,
    const dom_models_ethernet_link_config_t* config
) {
    if (!self || !self->ctx || !config) {
        return bad_argument_error("missing ethernet link config");
    }

    inf_device_ethernet_esp_w5500_impl_ctx_t* ctx = self->ctx;
    if (!ctx->eth_handle) {
        return bad_argument_error("ethernet is not initialized");
    }

    eth_speed_t  speed;
    eth_duplex_t duplex;
    if (!config->autoneg && (!speed_from_domain(config->speed, &speed) || !duplex_from_domain(config->duplex, &duplex))) {
        return bad_argument_error("invalid ethernet fixed link config");
    }

    bool      autoneg = config->autoneg;
    esp_err_t err     = esp_eth_ioctl(ctx->eth_handle, ETH_CMD_S_AUTONEGO, &autoneg);
    if (err != ESP_OK) {
        return error_from_esp("esp_eth_ioctl ETH_CMD_S_AUTONEGO", err);
    }

    if (config->autoneg) {
        return NULL;
    }

    err = esp_eth_ioctl(ctx->eth_handle, ETH_CMD_S_SPEED, &speed);
    if (err != ESP_OK) {
        return error_from_esp("esp_eth_ioctl ETH_CMD_S_SPEED", err);
    }

    err = esp_eth_ioctl(ctx->eth_handle, ETH_CMD_S_DUPLEX_MODE, &duplex);
    if (err != ESP_OK) {
        return error_from_esp("esp_eth_ioctl ETH_CMD_S_DUPLEX_MODE", err);
    }

    return NULL;
}

static dom_models_error_t* set_promiscuous_impl(
    dom_contracts_device_ethernet_t* self,
    bool                             enabled
) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing ethernet context");
    }

    inf_device_ethernet_esp_w5500_impl_ctx_t* ctx = self->ctx;
    if (!ctx->eth_handle) {
        return bad_argument_error("ethernet is not initialized");
    }

    esp_err_t err = esp_eth_ioctl(ctx->eth_handle, ETH_CMD_S_PROMISCUOUS, &enabled);
    if (err != ESP_OK) {
        return error_from_esp("esp_eth_ioctl ETH_CMD_S_PROMISCUOUS", err);
    }

    ctx->promiscuous = enabled;

    return NULL;
}

static dom_models_error_t* set_flow_control_impl(
    dom_contracts_device_ethernet_t* self,
    bool                             enabled
) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing ethernet context");
    }

    (void)enabled;

    return not_supported_error("w5500 flow control is not supported");
}

static dom_models_error_t* set_phy_loopback_impl(
    dom_contracts_device_ethernet_t* self,
    bool                             enabled
) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing ethernet context");
    }

    (void)enabled;

    return not_supported_error("w5500 phy loopback is not supported");
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

static void on_eth_start(inf_device_ethernet_esp_w5500_impl_ctx_t* ctx) {
    if (!ctx) {
        return;
    }

    ctx->started = true;
}

static void on_eth_stop(inf_device_ethernet_esp_w5500_impl_ctx_t* ctx) {
    if (!ctx) {
        return;
    }

    ctx->started = false;
    ctx->link_up = false;
}

static void on_eth_connected(inf_device_ethernet_esp_w5500_impl_ctx_t* ctx) {
    if (!ctx) {
        return;
    }

    ctx->link_up = true;
}

static void on_eth_disconnected(inf_device_ethernet_esp_w5500_impl_ctx_t* ctx) {
    if (!ctx) {
        return;
    }

    ctx->link_up = false;
}

/* Helper Function Implementations */

static dom_models_error_t* bad_argument_error(const char* msg) {
    return dom_models_error_new(ESP_W5500_IMPL_ACTOR, DOMAIN_MODELS_ERROR_TYPE_BAD_ARGUMENT, "%s", msg ? msg : "bad argument");
}

static dom_models_error_t* not_supported_error(const char* msg) {
    return dom_models_error_new(ESP_W5500_IMPL_ACTOR, DOMAIN_MODELS_ERROR_TYPE_NOT_SUPPORTED, "%s", msg ? msg : "not supported");
}

static dom_models_error_t* error_from_esp(const char* action, esp_err_t err) {
    if (err == ESP_OK) {
        return NULL;
    }

    if (err == ESP_ERR_NOT_SUPPORTED) {
        return dom_models_error_new(ESP_W5500_IMPL_ACTOR, DOMAIN_MODELS_ERROR_TYPE_NOT_SUPPORTED, "%s failed: esp_err=0x%x", action ? action : "esp_w5500", (unsigned int)err);
    }

    return dom_models_error_new(ESP_W5500_IMPL_ACTOR, DOMAIN_MODELS_ERROR_TYPE_SYSTEM_FAILURE, "%s failed: esp_err=0x%x", action ? action : "esp_w5500", (unsigned int)err);
}

static dom_models_error_t* append_error_from_esp(dom_models_error_t* handle, const char* action, esp_err_t err) {
    if (err == ESP_OK) {
        return handle;
    }

    if (!handle) {
        return error_from_esp(action, err);
    }

    return dom_models_error_append(handle, ESP_W5500_IMPL_ACTOR, DOMAIN_MODELS_ERROR_TYPE_SYSTEM_FAILURE, "%s failed: esp_err=0x%x", action ? action : "esp_w5500", (unsigned int)err);
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

static bool validate_config(const inf_device_ethernet_esp_w5500_impl_cfg_t* cfg) {
    if (!cfg) {
        return false;
    }

    if (cfg->spi_cs_gpio_num < 0 || cfg->spi_clock_hz <= 0 || cfg->spi_queue_size <= 0) {
        return false;
    }

    return (cfg->int_gpio_num >= 0 && cfg->poll_period_ms == 0) || (cfg->int_gpio_num < 0 && cfg->poll_period_ms > 0);
}

static bool speed_from_domain(dom_models_ethernet_speed_t in, eth_speed_t* out) {
    if (!out) {
        return false;
    }

    switch (in) {
        case DOM_MODELS_ETHERNET_SPEED_10M:
            *out = ETH_SPEED_10M;
            return true;
        case DOM_MODELS_ETHERNET_SPEED_100M:
            *out = ETH_SPEED_100M;
            return true;
        default:
            return false;
    }
}

static dom_models_ethernet_speed_t speed_to_domain(eth_speed_t in) {
    switch (in) {
        case ETH_SPEED_10M:
            return DOM_MODELS_ETHERNET_SPEED_10M;
        case ETH_SPEED_100M:
            return DOM_MODELS_ETHERNET_SPEED_100M;
        default:
            return DOM_MODELS_ETHERNET_SPEED_UNKNOWN;
    }
}

static bool duplex_from_domain(dom_models_ethernet_duplex_t in, eth_duplex_t* out) {
    if (!out) {
        return false;
    }

    switch (in) {
        case DOM_MODELS_ETHERNET_DUPLEX_HALF:
            *out = ETH_DUPLEX_HALF;
            return true;
        case DOM_MODELS_ETHERNET_DUPLEX_FULL:
            *out = ETH_DUPLEX_FULL;
            return true;
        default:
            return false;
    }
}

static dom_models_ethernet_duplex_t duplex_to_domain(eth_duplex_t in) {
    switch (in) {
        case ETH_DUPLEX_HALF:
            return DOM_MODELS_ETHERNET_DUPLEX_HALF;
        case ETH_DUPLEX_FULL:
            return DOM_MODELS_ETHERNET_DUPLEX_FULL;
        default:
            return DOM_MODELS_ETHERNET_DUPLEX_UNKNOWN;
    }
}

static void clear_runtime(inf_device_ethernet_esp_w5500_impl_ctx_t* ctx) {
    if (!ctx) {
        return;
    }

    ctx->started      = false;
    ctx->link_up      = false;
    ctx->promiscuous  = false;
    ctx->flow_control = false;
    ctx->phy_loopback = false;
}
