#include "infrastructure/network/interface/esp_netif_impl.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "domain/contracts/network/interface.h"
#include "domain/models/error.h"
#include "domain/models/network.h"
#include "esp_err.h"
#include "esp_eth_driver.h"
#include "esp_eth_netif_glue.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi_default.h"

#define ESP_NETIF_IMPL_ACTOR "esp_netif"

/* Helper Function Prototypes */

static dom_models_error_t*                           bad_argument_error(const char* msg);
static dom_models_error_t*                           bad_state_error(const char* msg);
static dom_models_error_t*                           not_supported_error(const char* msg);
static dom_models_error_t*                           system_failure_error(const char* msg);
static dom_models_error_type_t                       error_type_from_esp(esp_err_t err);
static dom_models_error_t*                           error_from_esp(const char* action, esp_err_t err);
static dom_models_error_t*                           append_error_from_esp(dom_models_error_t* handle, const char* action, esp_err_t err);
static size_t                                        bounded_strlen(const char* value, size_t max_len);
static void                                          copy_cstr(char* dst, size_t dst_size, const char* src);
static bool                                          valid_if_key(const char* if_key);
static uint32_t                                      flags_to_domain(esp_netif_flags_t flags);
static dom_models_network_dhcp_status_t              dhcp_status_to_domain(esp_netif_dhcp_status_t status);
static dom_models_network_interface_type_t           infer_type(const char* if_key, const char* desc, uint32_t flags);
static void                                          copy_ip4_addr(uint8_t out[DOM_MODELS_NETWORK_IPV4_LEN], const esp_ip4_addr_t* in);
static void                                          copy_ip4_info(dom_models_network_ipv4_info_t* out, const esp_netif_ip_info_t* in);
static void                                          copy_ip6_addr(dom_models_network_ipv6_addr_t* out, const esp_ip6_addr_t* in);
static void                                          copy_ip_addr(dom_models_network_ip_addr_t* out, const esp_ip_addr_t* in);
static void                                          load_dns(esp_netif_t* netif, dom_models_network_interface_t* out);
static void                                          load_ipv6(esp_netif_t* netif, dom_models_network_interface_t* out);
static void                                          fill_interface(esp_netif_t* netif, dom_models_network_interface_t* out);
static inf_network_interface_esp_netif_impl_entry_t* find_entry(inf_network_interface_esp_netif_impl_ctx_t* ctx, const char* if_key);
static inf_network_interface_esp_netif_impl_entry_t* find_free_entry(inf_network_interface_esp_netif_impl_ctx_t* ctx);
static inf_network_interface_esp_netif_impl_entry_t* latest_entry(inf_network_interface_esp_netif_impl_ctx_t* ctx);
static void                                          remove_entry(inf_network_interface_esp_netif_impl_ctx_t* ctx, inf_network_interface_esp_netif_impl_entry_t* entry);
static dom_models_error_t*                           destroy_entry_resources(inf_network_interface_esp_netif_impl_entry_t* entry);
static dom_models_error_t*                           create_wifi_interface(inf_network_interface_esp_netif_impl_ctx_t* ctx, const char* if_key, inf_network_interface_esp_netif_impl_kind_t kind);
static dom_models_error_t*                           create_ethernet_interface(inf_network_interface_esp_netif_impl_ctx_t* ctx, const char* if_key);
static void                                          store_entry(inf_network_interface_esp_netif_impl_entry_t* entry, const char* if_key, esp_netif_t* netif, inf_network_interface_esp_netif_impl_kind_t kind);

/* Contract Function Prototypes */

static dom_models_error_t* create_wifi_sta_impl(
    dom_contracts_network_interface_t* self
);
static dom_models_error_t* create_wifi_ap_impl(
    dom_contracts_network_interface_t* self
);
static dom_models_error_t* create_ethernet_impl(
    dom_contracts_network_interface_t* self
);
static dom_models_error_t* attach_ethernet_impl(
    dom_contracts_network_interface_t* self,
    const char*                        if_key,
    void*                              driver_handle
);
static dom_models_error_t* destroy_impl(
    dom_contracts_network_interface_t* self,
    const char*                        if_key
);
static dom_models_error_t* set_hostname_impl(
    dom_contracts_network_interface_t* self,
    const char*                        if_key,
    const char*                        hostname
);
static dom_models_error_t* start_dhcp_client_impl(
    dom_contracts_network_interface_t* self,
    const char*                        if_key
);
static dom_models_error_t* stop_dhcp_client_impl(
    dom_contracts_network_interface_t* self,
    const char*                        if_key
);
static dom_models_error_t* start_dhcp_server_impl(
    dom_contracts_network_interface_t* self,
    const char*                        if_key
);
static dom_models_error_t* stop_dhcp_server_impl(
    dom_contracts_network_interface_t* self,
    const char*                        if_key
);
static dom_models_error_t* get_interface_impl(
    dom_contracts_network_interface_t* self,
    const char*                        if_key,
    dom_models_network_interface_t*    out
);
static dom_models_error_t* get_interfaces_impl(
    dom_contracts_network_interface_t* self,
    dom_models_network_t*              out
);

/* Constructor and Destructor */

dom_contracts_network_interface_t* inf_network_interface_esp_netif_impl_new(const inf_network_interface_esp_netif_impl_cfg_t* cfg) {
    inf_network_interface_esp_netif_impl_ctx_t* ctx = (inf_network_interface_esp_netif_impl_ctx_t*)calloc(1, sizeof(inf_network_interface_esp_netif_impl_ctx_t));
    if (!ctx) {
        return NULL;
    }

    if (!cfg) {
        inf_network_interface_esp_netif_impl_cfg_t default_cfg = INF_NETWORK_INTERFACE_ESP_NETIF_IMPL_CFG_DEFAULT();
        memcpy(&ctx->cfg, &default_cfg, sizeof(inf_network_interface_esp_netif_impl_cfg_t));
    } else {
        memcpy(&ctx->cfg, cfg, sizeof(inf_network_interface_esp_netif_impl_cfg_t));
    }

    dom_contracts_network_interface_t* self = dom_contracts_network_interface_new(ctx);
    if (!self) {
        free(ctx);
        return NULL;
    }

    self->create_wifi_sta   = create_wifi_sta_impl;
    self->create_wifi_ap    = create_wifi_ap_impl;
    self->create_ethernet   = create_ethernet_impl;
    self->attach_ethernet   = attach_ethernet_impl;
    self->destroy           = destroy_impl;
    self->set_hostname      = set_hostname_impl;
    self->start_dhcp_client = start_dhcp_client_impl;
    self->stop_dhcp_client  = stop_dhcp_client_impl;
    self->start_dhcp_server = start_dhcp_server_impl;
    self->stop_dhcp_server  = stop_dhcp_server_impl;
    self->get_interface     = get_interface_impl;
    self->get_interfaces    = get_interfaces_impl;

    return self;
}

void inf_network_interface_esp_netif_impl_delete(dom_contracts_network_interface_t* self) {
    if (!self || !self->ctx) {
        return;
    }

    inf_network_interface_esp_netif_impl_ctx_t* ctx = self->ctx;

    dom_models_error_t* err = inf_network_interface_esp_netif_impl_deinit(self);
    dom_models_error_delete(err);

    free(ctx);
    dom_contracts_network_interface_delete(self);
}

dom_models_error_t* inf_network_interface_esp_netif_impl_init(dom_contracts_network_interface_t* self) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing network interface context");
    }

    inf_network_interface_esp_netif_impl_ctx_t* ctx = self->ctx;

    if (ctx->cfg.init_netif) {
        esp_err_t err = esp_netif_init();
        if (err != ESP_OK) {
            return error_from_esp("esp_netif_init", err);
        }
        ctx->netif_initialized = true;
    }

    if (ctx->cfg.create_default_event_loop) {
        esp_err_t err = esp_event_loop_create_default();
        if (err == ESP_OK) {
            ctx->default_event_loop_created = true;
        } else if (err != ESP_ERR_INVALID_STATE) {
            return error_from_esp("esp_event_loop_create_default", err);
        }
    }

    return NULL;
}

dom_models_error_t* inf_network_interface_esp_netif_impl_deinit(dom_contracts_network_interface_t* self) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing network interface context");
    }

    inf_network_interface_esp_netif_impl_ctx_t* ctx        = self->ctx;
    dom_models_error_t*                         err_handle = NULL;

    for (size_t i = 0; i < DOM_MODELS_NETWORK_MAX_INTERFACES; i++) {
        if (!ctx->entries[i].used) {
            continue;
        }

        dom_models_error_t* err = destroy_entry_resources(&ctx->entries[i]);
        if (err) {
            err_handle = dom_models_error_append(err_handle, ESP_NETIF_IMPL_ACTOR, err->type, "%s", err->msg ? err->msg : "failed to destroy network interface");
            dom_models_error_delete(err);
        }

        memset(&ctx->entries[i], 0, sizeof(inf_network_interface_esp_netif_impl_entry_t));
    }

    if (ctx->cfg.delete_default_event_loop_on_deinit && ctx->default_event_loop_created) {
        esp_err_t err = esp_event_loop_delete_default();
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
            err_handle = append_error_from_esp(err_handle, "esp_event_loop_delete_default", err);
        } else {
            ctx->default_event_loop_created = false;
        }
    }

    ctx->netif_initialized = false;

    return err_handle;
}

/* Contract Function Implementations */

static dom_models_error_t* create_wifi_sta_impl(
    dom_contracts_network_interface_t* self
) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing network interface context");
    }

    inf_network_interface_esp_netif_impl_ctx_t* ctx = self->ctx;

    return create_wifi_interface(ctx, ctx->cfg.sta_if_key, INF_NETWORK_INTERFACE_ESP_NETIF_IMPL_KIND_WIFI_STA);
}

static dom_models_error_t* create_wifi_ap_impl(
    dom_contracts_network_interface_t* self
) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing network interface context");
    }

    inf_network_interface_esp_netif_impl_ctx_t* ctx = self->ctx;

    return create_wifi_interface(ctx, ctx->cfg.ap_if_key, INF_NETWORK_INTERFACE_ESP_NETIF_IMPL_KIND_WIFI_AP);
}

static dom_models_error_t* create_ethernet_impl(
    dom_contracts_network_interface_t* self
) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing network interface context");
    }

    inf_network_interface_esp_netif_impl_ctx_t* ctx = self->ctx;

    return create_ethernet_interface(ctx, ctx->cfg.eth_if_key);
}

static dom_models_error_t* attach_ethernet_impl(
    dom_contracts_network_interface_t* self,
    const char*                        if_key,
    void*                              driver_handle
) {
    if (!self || !self->ctx || !if_key || !driver_handle) {
        return bad_argument_error("missing ethernet attach argument");
    }

    inf_network_interface_esp_netif_impl_ctx_t*   ctx   = self->ctx;
    inf_network_interface_esp_netif_impl_entry_t* entry = find_entry(ctx, if_key);
    if (!entry) {
        return bad_argument_error("network interface not found");
    }
    if (entry->kind != INF_NETWORK_INTERFACE_ESP_NETIF_IMPL_KIND_ETHERNET) {
        return bad_argument_error("network interface is not ethernet");
    }
    if (!entry->netif) {
        return bad_state_error("ethernet netif is not created");
    }
    if (entry->eth_glue) {
        return bad_state_error("ethernet netif is already attached");
    }

    esp_eth_netif_glue_handle_t glue = esp_eth_new_netif_glue((esp_eth_handle_t)driver_handle);
    if (!glue) {
        return system_failure_error("esp_eth_new_netif_glue failed");
    }

    esp_err_t err = esp_netif_attach(entry->netif, glue);
    if (err != ESP_OK) {
        esp_eth_del_netif_glue(glue);
        return error_from_esp("esp_netif_attach", err);
    }

    entry->eth_glue = glue;

    return NULL;
}

static dom_models_error_t* destroy_impl(
    dom_contracts_network_interface_t* self,
    const char*                        if_key
) {
    if (!self || !self->ctx || !if_key) {
        return bad_argument_error("missing network interface destroy argument");
    }

    inf_network_interface_esp_netif_impl_ctx_t*   ctx   = self->ctx;
    inf_network_interface_esp_netif_impl_entry_t* entry = find_entry(ctx, if_key);
    if (!entry) {
        return bad_argument_error("managed network interface not found");
    }

    dom_models_error_t* err = destroy_entry_resources(entry);
    remove_entry(ctx, entry);

    return err;
}

static dom_models_error_t* set_hostname_impl(
    dom_contracts_network_interface_t* self,
    const char*                        if_key,
    const char*                        hostname
) {
    if (!self || !self->ctx || !if_key || !hostname) {
        return bad_argument_error("missing network interface hostname argument");
    }

    esp_netif_t* netif = esp_netif_get_handle_from_ifkey(if_key);
    if (!netif) {
        return bad_argument_error("network interface not found");
    }

    esp_err_t err = esp_netif_set_hostname(netif, hostname);
    if (err != ESP_OK) {
        return error_from_esp("esp_netif_set_hostname", err);
    }

    return NULL;
}

static dom_models_error_t* start_dhcp_client_impl(
    dom_contracts_network_interface_t* self,
    const char*                        if_key
) {
    if (!self || !self->ctx || !if_key) {
        return bad_argument_error("missing network interface dhcp client argument");
    }

    esp_netif_t* netif = esp_netif_get_handle_from_ifkey(if_key);
    if (!netif) {
        return bad_argument_error("network interface not found");
    }
    if ((esp_netif_get_flags(netif) & ESP_NETIF_DHCP_CLIENT) == 0) {
        return not_supported_error("dhcp client is not supported");
    }

    esp_err_t err = esp_netif_dhcpc_start(netif);
    if (err != ESP_OK && err != ESP_ERR_ESP_NETIF_DHCP_ALREADY_STARTED) {
        return error_from_esp("esp_netif_dhcpc_start", err);
    }

    return NULL;
}

static dom_models_error_t* stop_dhcp_client_impl(
    dom_contracts_network_interface_t* self,
    const char*                        if_key
) {
    if (!self || !self->ctx || !if_key) {
        return bad_argument_error("missing network interface dhcp client argument");
    }

    esp_netif_t* netif = esp_netif_get_handle_from_ifkey(if_key);
    if (!netif) {
        return bad_argument_error("network interface not found");
    }
    if ((esp_netif_get_flags(netif) & ESP_NETIF_DHCP_CLIENT) == 0) {
        return not_supported_error("dhcp client is not supported");
    }

    esp_err_t err = esp_netif_dhcpc_stop(netif);
    if (err != ESP_OK && err != ESP_ERR_ESP_NETIF_DHCP_ALREADY_STOPPED) {
        return error_from_esp("esp_netif_dhcpc_stop", err);
    }

    return NULL;
}

static dom_models_error_t* start_dhcp_server_impl(
    dom_contracts_network_interface_t* self,
    const char*                        if_key
) {
    if (!self || !self->ctx || !if_key) {
        return bad_argument_error("missing network interface dhcp server argument");
    }

    esp_netif_t* netif = esp_netif_get_handle_from_ifkey(if_key);
    if (!netif) {
        return bad_argument_error("network interface not found");
    }
    if ((esp_netif_get_flags(netif) & ESP_NETIF_DHCP_SERVER) == 0) {
        return not_supported_error("dhcp server is not supported");
    }

    esp_err_t err = esp_netif_dhcps_start(netif);
    if (err != ESP_OK && err != ESP_ERR_ESP_NETIF_DHCP_ALREADY_STARTED) {
        return error_from_esp("esp_netif_dhcps_start", err);
    }

    return NULL;
}

static dom_models_error_t* stop_dhcp_server_impl(
    dom_contracts_network_interface_t* self,
    const char*                        if_key
) {
    if (!self || !self->ctx || !if_key) {
        return bad_argument_error("missing network interface dhcp server argument");
    }

    esp_netif_t* netif = esp_netif_get_handle_from_ifkey(if_key);
    if (!netif) {
        return bad_argument_error("network interface not found");
    }
    if ((esp_netif_get_flags(netif) & ESP_NETIF_DHCP_SERVER) == 0) {
        return not_supported_error("dhcp server is not supported");
    }

    esp_err_t err = esp_netif_dhcps_stop(netif);
    if (err != ESP_OK && err != ESP_ERR_ESP_NETIF_DHCP_ALREADY_STOPPED) {
        return error_from_esp("esp_netif_dhcps_stop", err);
    }

    return NULL;
}

static dom_models_error_t* get_interface_impl(
    dom_contracts_network_interface_t* self,
    const char*                        if_key,
    dom_models_network_interface_t*    out
) {
    if (!self || !self->ctx || !if_key || !out) {
        return bad_argument_error("missing network interface get argument");
    }

    esp_netif_t* netif = esp_netif_get_handle_from_ifkey(if_key);
    if (!netif) {
        return bad_argument_error("network interface not found");
    }

    fill_interface(netif, out);

    return NULL;
}

static dom_models_error_t* get_interfaces_impl(
    dom_contracts_network_interface_t* self,
    dom_models_network_t*              out
) {
    if (!self || !self->ctx || !out) {
        return bad_argument_error("missing network interfaces get argument");
    }

    memset(out, 0, sizeof(dom_models_network_t));
    out->total_count = esp_netif_get_nr_of_ifs();

    esp_netif_t* netif = NULL;
    while ((netif = esp_netif_next_unsafe(netif)) != NULL) {
        if (out->count < DOM_MODELS_NETWORK_MAX_INTERFACES) {
            fill_interface(netif, &out->interfaces[out->count]);
            out->count++;
        } else {
            out->truncated = true;
        }
    }

    if (out->count < out->total_count) {
        out->truncated = true;
    }

    return NULL;
}

/* Helper Function Implementations */

static dom_models_error_t* bad_argument_error(const char* msg) {
    return dom_models_error_new(ESP_NETIF_IMPL_ACTOR, DOMAIN_MODELS_ERROR_TYPE_BAD_ARGUMENT, "%s", msg);
}

static dom_models_error_t* bad_state_error(const char* msg) {
    return dom_models_error_new(ESP_NETIF_IMPL_ACTOR, DOMAIN_MODELS_ERROR_TYPE_BAD_STATE, "%s", msg);
}

static dom_models_error_t* not_supported_error(const char* msg) {
    return dom_models_error_new(ESP_NETIF_IMPL_ACTOR, DOMAIN_MODELS_ERROR_TYPE_NOT_SUPPORTED, "%s", msg);
}

static dom_models_error_t* system_failure_error(const char* msg) {
    return dom_models_error_new(ESP_NETIF_IMPL_ACTOR, DOMAIN_MODELS_ERROR_TYPE_SYSTEM_FAILURE, "%s", msg);
}

static dom_models_error_type_t error_type_from_esp(esp_err_t err) {
    switch (err) {
        case ESP_ERR_NO_MEM:
        case ESP_ERR_ESP_NETIF_NO_MEM:
            return DOMAIN_MODELS_ERROR_TYPE_MALLOC_FAILED;
        case ESP_ERR_INVALID_ARG:
        case ESP_ERR_ESP_NETIF_INVALID_PARAMS:
            return DOMAIN_MODELS_ERROR_TYPE_BAD_ARGUMENT;
        case ESP_ERR_INVALID_STATE:
        case ESP_ERR_ESP_NETIF_DHCP_ALREADY_STARTED:
        case ESP_ERR_ESP_NETIF_DHCP_ALREADY_STOPPED:
        case ESP_ERR_ESP_NETIF_DHCP_NOT_STOPPED:
        case ESP_ERR_ESP_NETIF_IF_NOT_READY:
            return DOMAIN_MODELS_ERROR_TYPE_BAD_STATE;
        case ESP_ERR_NOT_SUPPORTED:
            return DOMAIN_MODELS_ERROR_TYPE_NOT_SUPPORTED;
        case ESP_ERR_TIMEOUT:
            return DOMAIN_MODELS_ERROR_TYPE_TIMEOUT;
        default:
            return DOMAIN_MODELS_ERROR_TYPE_SYSTEM_FAILURE;
    }
}

static dom_models_error_t* error_from_esp(const char* action, esp_err_t err) {
    return dom_models_error_new(ESP_NETIF_IMPL_ACTOR, error_type_from_esp(err), "%s failed: %s (%d)", action, esp_err_to_name(err), (int)err);
}

static dom_models_error_t* append_error_from_esp(dom_models_error_t* handle, const char* action, esp_err_t err) {
    if (!handle) {
        return error_from_esp(action, err);
    }

    return dom_models_error_append(handle, ESP_NETIF_IMPL_ACTOR, error_type_from_esp(err), "%s failed: %s (%d)", action, esp_err_to_name(err), (int)err);
}

static size_t bounded_strlen(const char* value, size_t max_len) {
    size_t len = 0;
    while (len < max_len && value && value[len] != '\0') {
        len++;
    }
    return len;
}

static void copy_cstr(char* dst, size_t dst_size, const char* src) {
    if (!dst || dst_size == 0) {
        return;
    }

    size_t src_len  = bounded_strlen(src, dst_size - 1);
    size_t copy_len = src_len < dst_size - 1 ? src_len : dst_size - 1;

    if (copy_len > 0) {
        memcpy(dst, src, copy_len);
    }
    dst[copy_len] = '\0';
}

static bool valid_if_key(const char* if_key) {
    size_t len = bounded_strlen(if_key, DOM_MODELS_NETWORK_IF_KEY_LEN);

    return len > 0 && len < DOM_MODELS_NETWORK_IF_KEY_LEN;
}

static uint32_t flags_to_domain(esp_netif_flags_t flags) {
    uint32_t out = 0;

    if (flags & ESP_NETIF_DHCP_CLIENT) {
        out |= DOM_MODELS_NETWORK_INTERFACE_FLAG_DHCP_CLIENT;
    }
    if (flags & ESP_NETIF_DHCP_SERVER) {
        out |= DOM_MODELS_NETWORK_INTERFACE_FLAG_DHCP_SERVER;
    }
    if (flags & ESP_NETIF_FLAG_AUTOUP) {
        out |= DOM_MODELS_NETWORK_INTERFACE_FLAG_AUTO_UP;
    }
    if (flags & ESP_NETIF_FLAG_GARP) {
        out |= DOM_MODELS_NETWORK_INTERFACE_FLAG_GARP;
    }
    if (flags & ESP_NETIF_FLAG_EVENT_IP_MODIFIED) {
        out |= DOM_MODELS_NETWORK_INTERFACE_FLAG_EVENT_IP_MODIFIED;
    }
    if (flags & ESP_NETIF_FLAG_IS_PPP) {
        out |= DOM_MODELS_NETWORK_INTERFACE_FLAG_PPP;
    }
    if (flags & ESP_NETIF_FLAG_IS_BRIDGE) {
        out |= DOM_MODELS_NETWORK_INTERFACE_FLAG_BRIDGE;
    }
    if (flags & ESP_NETIF_FLAG_MLDV6_REPORT) {
        out |= DOM_MODELS_NETWORK_INTERFACE_FLAG_MLDV6_REPORT;
    }
    if (flags & ESP_NETIF_FLAG_IPV6_AUTOCONFIG_ENABLED) {
        out |= DOM_MODELS_NETWORK_INTERFACE_FLAG_IPV6_AUTOCONFIG;
    }

    return out;
}

static dom_models_network_dhcp_status_t dhcp_status_to_domain(esp_netif_dhcp_status_t status) {
    switch (status) {
        case ESP_NETIF_DHCP_INIT:
            return DOM_MODELS_NETWORK_DHCP_STATUS_INIT;
        case ESP_NETIF_DHCP_STARTED:
            return DOM_MODELS_NETWORK_DHCP_STATUS_STARTED;
        case ESP_NETIF_DHCP_STOPPED:
            return DOM_MODELS_NETWORK_DHCP_STATUS_STOPPED;
        default:
            return DOM_MODELS_NETWORK_DHCP_STATUS_UNKNOWN;
    }
}

static dom_models_network_interface_type_t infer_type(const char* if_key, const char* desc, uint32_t flags) {
    if (flags & DOM_MODELS_NETWORK_INTERFACE_FLAG_PPP) {
        return DOM_MODELS_NETWORK_INTERFACE_TYPE_PPP;
    }
    if (flags & DOM_MODELS_NETWORK_INTERFACE_FLAG_BRIDGE) {
        return DOM_MODELS_NETWORK_INTERFACE_TYPE_BRIDGE;
    }
    if ((if_key && strcmp(if_key, "WIFI_STA_DEF") == 0) || (desc && strcmp(desc, "sta") == 0)) {
        return DOM_MODELS_NETWORK_INTERFACE_TYPE_WIFI_STA;
    }
    if ((if_key && strcmp(if_key, "WIFI_AP_DEF") == 0) || (desc && strcmp(desc, "ap") == 0)) {
        return DOM_MODELS_NETWORK_INTERFACE_TYPE_WIFI_AP;
    }
    if ((if_key && strcmp(if_key, "ETH_DEF") == 0) || (desc && strcmp(desc, "eth") == 0)) {
        return DOM_MODELS_NETWORK_INTERFACE_TYPE_ETHERNET;
    }
    if (desc && strcmp(desc, "lo") == 0) {
        return DOM_MODELS_NETWORK_INTERFACE_TYPE_LOOPBACK;
    }

    return DOM_MODELS_NETWORK_INTERFACE_TYPE_UNKNOWN;
}

static void copy_ip4_addr(uint8_t out[DOM_MODELS_NETWORK_IPV4_LEN], const esp_ip4_addr_t* in) {
    out[0] = esp_ip4_addr_get_byte(in, 0);
    out[1] = esp_ip4_addr_get_byte(in, 1);
    out[2] = esp_ip4_addr_get_byte(in, 2);
    out[3] = esp_ip4_addr_get_byte(in, 3);
}

static void copy_ip4_info(dom_models_network_ipv4_info_t* out, const esp_netif_ip_info_t* in) {
    out->available = true;
    copy_ip4_addr(out->ip, &in->ip);
    copy_ip4_addr(out->netmask, &in->netmask);
    copy_ip4_addr(out->gateway, &in->gw);
}

static void copy_ip6_addr(dom_models_network_ipv6_addr_t* out, const esp_ip6_addr_t* in) {
    out->available = true;
    out->zone      = in->zone;

    for (size_t i = 0; i < 4; i++) {
        uint32_t word        = esp_netif_htonl(in->addr[i]);
        out->addr[i * 4]     = (uint8_t)((word >> 24) & 0xff);
        out->addr[i * 4 + 1] = (uint8_t)((word >> 16) & 0xff);
        out->addr[i * 4 + 2] = (uint8_t)((word >> 8) & 0xff);
        out->addr[i * 4 + 3] = (uint8_t)(word & 0xff);
    }
}

static void copy_ip_addr(dom_models_network_ip_addr_t* out, const esp_ip_addr_t* in) {
    memset(out, 0, sizeof(dom_models_network_ip_addr_t));

    if (in->type == ESP_IPADDR_TYPE_V4) {
        out->family = DOM_MODELS_NETWORK_IP_FAMILY_IPV4;
        copy_ip4_addr(out->bytes, &in->u_addr.ip4);
    } else if (in->type == ESP_IPADDR_TYPE_V6) {
        out->family = DOM_MODELS_NETWORK_IP_FAMILY_IPV6;
        out->zone   = in->u_addr.ip6.zone;
        for (size_t i = 0; i < 4; i++) {
            uint32_t word         = esp_netif_htonl(in->u_addr.ip6.addr[i]);
            out->bytes[i * 4]     = (uint8_t)((word >> 24) & 0xff);
            out->bytes[i * 4 + 1] = (uint8_t)((word >> 16) & 0xff);
            out->bytes[i * 4 + 2] = (uint8_t)((word >> 8) & 0xff);
            out->bytes[i * 4 + 3] = (uint8_t)(word & 0xff);
        }
    } else {
        out->family = DOM_MODELS_NETWORK_IP_FAMILY_NONE;
    }
}

static void load_dns(esp_netif_t* netif, dom_models_network_interface_t* out) {
    static const esp_netif_dns_type_t dns_types[DOM_MODELS_NETWORK_DNS_MAX] = {
        ESP_NETIF_DNS_MAIN,
        ESP_NETIF_DNS_BACKUP,
        ESP_NETIF_DNS_FALLBACK,
    };

    for (size_t i = 0; i < DOM_MODELS_NETWORK_DNS_MAX; i++) {
        esp_netif_dns_info_t dns;
        memset(&dns, 0, sizeof(esp_netif_dns_info_t));

        esp_err_t err = esp_netif_get_dns_info(netif, dns_types[i], &dns);
        if (err != ESP_OK) {
            continue;
        }

        out->dns[i].available = true;
        copy_ip_addr(&out->dns[i].addr, &dns.ip);
    }
}

static void load_ipv6(esp_netif_t* netif, dom_models_network_interface_t* out) {
#if CONFIG_LWIP_IPV6
    esp_ip6_addr_t addr;
    memset(&addr, 0, sizeof(esp_ip6_addr_t));

    esp_err_t err = esp_netif_get_ip6_linklocal(netif, &addr);
    if (err == ESP_OK) {
        copy_ip6_addr(&out->ipv6_linklocal, &addr);
    }

    memset(&addr, 0, sizeof(esp_ip6_addr_t));
    err = esp_netif_get_ip6_global(netif, &addr);
    if (err == ESP_OK) {
        copy_ip6_addr(&out->ipv6_global, &addr);
    }

    esp_ip6_addr_t addrs[CONFIG_LWIP_IPV6_NUM_ADDRESSES];
    memset(addrs, 0, sizeof(addrs));

    int count = esp_netif_get_all_ip6(netif, addrs);
    if (count > 0) {
        out->ipv6_count = (size_t)count;
        if (out->ipv6_count > DOM_MODELS_NETWORK_IPV6_MAX) {
            out->ipv6_count = DOM_MODELS_NETWORK_IPV6_MAX;
        }

        for (size_t i = 0; i < out->ipv6_count; i++) {
            copy_ip6_addr(&out->ipv6[i], &addrs[i]);
        }
    }

    memset(addrs, 0, sizeof(addrs));
    count = esp_netif_get_all_preferred_ip6(netif, addrs);
    if (count > 0) {
        out->preferred_ipv6_count = (size_t)count;
        if (out->preferred_ipv6_count > DOM_MODELS_NETWORK_IPV6_MAX) {
            out->preferred_ipv6_count = DOM_MODELS_NETWORK_IPV6_MAX;
        }

        for (size_t i = 0; i < out->preferred_ipv6_count; i++) {
            copy_ip6_addr(&out->preferred_ipv6[i], &addrs[i]);
        }
    }
#else
    (void)netif;
    (void)out;
#endif
}

static void fill_interface(esp_netif_t* netif, dom_models_network_interface_t* out) {
    memset(out, 0, sizeof(dom_models_network_interface_t));

    const char* if_key = esp_netif_get_ifkey(netif);
    const char* desc   = esp_netif_get_desc(netif);
    uint32_t    flags  = flags_to_domain(esp_netif_get_flags(netif));

    copy_cstr(out->if_key, sizeof(out->if_key), if_key);
    copy_cstr(out->desc, sizeof(out->desc), desc);

    out->type       = infer_type(if_key, desc, flags);
    out->flags      = flags;
    out->is_default = esp_netif_get_default_netif() == netif;
    out->is_up      = esp_netif_is_netif_up(netif);

    const char* hostname = NULL;
    esp_err_t   err      = esp_netif_get_hostname(netif, &hostname);
    if (err == ESP_OK && hostname && hostname[0] != '\0') {
        out->hostname_available = true;
        copy_cstr(out->hostname, sizeof(out->hostname), hostname);
    }

    err = esp_netif_get_netif_impl_name(netif, out->impl_name);
    if (err == ESP_OK) {
        out->impl_name_available = true;
    }

    err = esp_netif_get_mac(netif, out->mac);
    if (err == ESP_OK) {
        out->mac_available = true;
    }

    int impl_index = esp_netif_get_netif_impl_index(netif);
    if (impl_index >= 0) {
        out->impl_index_available = true;
        out->impl_index           = impl_index;
    }

    int route_prio = esp_netif_get_route_prio(netif);
    if (route_prio >= 0) {
        out->route_prio_available = true;
        out->route_prio           = route_prio;
    }

    uint16_t mtu = 0;
    err          = esp_netif_get_mtu(netif, &mtu);
    if (err == ESP_OK) {
        out->mtu_available = true;
        out->mtu           = mtu;
    }

    int32_t event_id = esp_netif_get_event_id(netif, ESP_NETIF_IP_EVENT_GOT_IP);
    if (event_id >= 0) {
        out->got_ip_event_id_available = true;
        out->got_ip_event_id           = event_id;
    }

    event_id = esp_netif_get_event_id(netif, ESP_NETIF_IP_EVENT_LOST_IP);
    if (event_id >= 0) {
        out->lost_ip_event_id_available = true;
        out->lost_ip_event_id           = event_id;
    }

    if (esp_netif_get_flags(netif) & ESP_NETIF_DHCP_CLIENT) {
        esp_netif_dhcp_status_t status;
        err = esp_netif_dhcpc_get_status(netif, &status);
        if (err == ESP_OK) {
            out->dhcp_client_status_available = true;
            out->dhcp_client_status           = dhcp_status_to_domain(status);
        }
    }

    if (esp_netif_get_flags(netif) & ESP_NETIF_DHCP_SERVER) {
        esp_netif_dhcp_status_t status;
        err = esp_netif_dhcps_get_status(netif, &status);
        if (err == ESP_OK) {
            out->dhcp_server_status_available = true;
            out->dhcp_server_status           = dhcp_status_to_domain(status);
        }
    }

    esp_netif_ip_info_t ip_info;
    memset(&ip_info, 0, sizeof(esp_netif_ip_info_t));
    err = esp_netif_get_ip_info(netif, &ip_info);
    if (err == ESP_OK) {
        copy_ip4_info(&out->ipv4, &ip_info);
    }

    memset(&ip_info, 0, sizeof(esp_netif_ip_info_t));
    err = esp_netif_get_old_ip_info(netif, &ip_info);
    if (err == ESP_OK) {
        copy_ip4_info(&out->old_ipv4, &ip_info);
    }

    load_dns(netif, out);
    load_ipv6(netif, out);
}

static inf_network_interface_esp_netif_impl_entry_t* find_entry(inf_network_interface_esp_netif_impl_ctx_t* ctx, const char* if_key) {
    if (!ctx || !if_key) {
        return NULL;
    }

    for (size_t i = 0; i < DOM_MODELS_NETWORK_MAX_INTERFACES; i++) {
        if (ctx->entries[i].used && strncmp(ctx->entries[i].if_key, if_key, sizeof(ctx->entries[i].if_key)) == 0) {
            return &ctx->entries[i];
        }
    }

    return NULL;
}

static inf_network_interface_esp_netif_impl_entry_t* find_free_entry(inf_network_interface_esp_netif_impl_ctx_t* ctx) {
    for (size_t i = 0; i < DOM_MODELS_NETWORK_MAX_INTERFACES; i++) {
        if (!ctx->entries[i].used) {
            return &ctx->entries[i];
        }
    }

    return NULL;
}

static inf_network_interface_esp_netif_impl_entry_t* latest_entry(inf_network_interface_esp_netif_impl_ctx_t* ctx) {
    for (size_t i = DOM_MODELS_NETWORK_MAX_INTERFACES; i > 0; i--) {
        if (ctx->entries[i - 1].used) {
            return &ctx->entries[i - 1];
        }
    }

    return NULL;
}

static void remove_entry(inf_network_interface_esp_netif_impl_ctx_t* ctx, inf_network_interface_esp_netif_impl_entry_t* entry) {
    inf_network_interface_esp_netif_impl_entry_t* latest = latest_entry(ctx);
    if (!latest || !entry) {
        return;
    }

    if (entry != latest) {
        memcpy(entry, latest, sizeof(inf_network_interface_esp_netif_impl_entry_t));
    }

    memset(latest, 0, sizeof(inf_network_interface_esp_netif_impl_entry_t));
}

static dom_models_error_t* destroy_entry_resources(inf_network_interface_esp_netif_impl_entry_t* entry) {
    dom_models_error_t* err_handle = NULL;

    if (!entry || !entry->used) {
        return NULL;
    }

    if (entry->kind == INF_NETWORK_INTERFACE_ESP_NETIF_IMPL_KIND_ETHERNET) {
        if (entry->eth_glue) {
            esp_err_t err = esp_eth_del_netif_glue(entry->eth_glue);
            if (err != ESP_OK) {
                err_handle = append_error_from_esp(err_handle, "esp_eth_del_netif_glue", err);
            } else {
                entry->eth_glue = NULL;
            }
        }

        if (entry->netif) {
            esp_netif_destroy(entry->netif);
            entry->netif = NULL;
        }
    } else if (entry->kind == INF_NETWORK_INTERFACE_ESP_NETIF_IMPL_KIND_WIFI_STA ||
               entry->kind == INF_NETWORK_INTERFACE_ESP_NETIF_IMPL_KIND_WIFI_AP) {
        if (entry->netif) {
            esp_err_t err = esp_wifi_clear_default_wifi_driver_and_handlers(entry->netif);
            if (err != ESP_OK) {
                err_handle = append_error_from_esp(err_handle, "esp_wifi_clear_default_wifi_driver_and_handlers", err);
            }
            esp_netif_destroy(entry->netif);
            entry->netif = NULL;
        }
    }

    return err_handle;
}

static dom_models_error_t* create_wifi_interface(inf_network_interface_esp_netif_impl_ctx_t* ctx, const char* if_key, inf_network_interface_esp_netif_impl_kind_t kind) {
    if (!valid_if_key(if_key)) {
        return bad_argument_error("invalid network interface key");
    }
    if (esp_netif_get_handle_from_ifkey(if_key)) {
        return bad_state_error("network interface already exists");
    }

    inf_network_interface_esp_netif_impl_entry_t* entry = find_free_entry(ctx);
    if (!entry) {
        return system_failure_error("network interface storage is full");
    }

    esp_netif_inherent_config_t base;
    memset(&base, 0, sizeof(esp_netif_inherent_config_t));

    esp_netif_config_t config;
    memset(&config, 0, sizeof(esp_netif_config_t));

    if (kind == INF_NETWORK_INTERFACE_ESP_NETIF_IMPL_KIND_WIFI_STA) {
        memcpy(&base, ESP_NETIF_BASE_DEFAULT_WIFI_STA, sizeof(esp_netif_inherent_config_t));
        base.if_key  = if_key;
        config.base  = &base;
        config.stack = ESP_NETIF_NETSTACK_DEFAULT_WIFI_STA;

        esp_netif_t* netif = esp_netif_new(&config);
        if (!netif) {
            return system_failure_error("esp_netif_new failed");
        }

        esp_err_t err = esp_netif_attach_wifi_station(netif);
        if (err != ESP_OK) {
            esp_netif_destroy(netif);
            return error_from_esp("esp_netif_attach_wifi_station", err);
        }

        err = esp_wifi_set_default_wifi_sta_handlers();
        if (err != ESP_OK) {
            esp_wifi_clear_default_wifi_driver_and_handlers(netif);
            esp_netif_destroy(netif);
            return error_from_esp("esp_wifi_set_default_wifi_sta_handlers", err);
        }

        store_entry(entry, if_key, netif, kind);
        return NULL;
    }

    if (kind == INF_NETWORK_INTERFACE_ESP_NETIF_IMPL_KIND_WIFI_AP) {
#ifdef CONFIG_ESP_WIFI_SOFTAP_SUPPORT
        memcpy(&base, ESP_NETIF_BASE_DEFAULT_WIFI_AP, sizeof(esp_netif_inherent_config_t));
        base.if_key  = if_key;
        config.base  = &base;
        config.stack = ESP_NETIF_NETSTACK_DEFAULT_WIFI_AP;

        esp_netif_t* netif = esp_netif_new(&config);
        if (!netif) {
            return system_failure_error("esp_netif_new failed");
        }

        esp_err_t err = esp_netif_attach_wifi_ap(netif);
        if (err != ESP_OK) {
            esp_netif_destroy(netif);
            return error_from_esp("esp_netif_attach_wifi_ap", err);
        }

        err = esp_wifi_set_default_wifi_ap_handlers();
        if (err != ESP_OK) {
            esp_wifi_clear_default_wifi_driver_and_handlers(netif);
            esp_netif_destroy(netif);
            return error_from_esp("esp_wifi_set_default_wifi_ap_handlers", err);
        }

        store_entry(entry, if_key, netif, kind);
        return NULL;
#else
        return not_supported_error("wifi ap is not supported");
#endif
    }

    return bad_argument_error("unsupported wifi interface type");
}

static dom_models_error_t* create_ethernet_interface(inf_network_interface_esp_netif_impl_ctx_t* ctx, const char* if_key) {
    if (!valid_if_key(if_key)) {
        return bad_argument_error("invalid network interface key");
    }
    if (esp_netif_get_handle_from_ifkey(if_key)) {
        return bad_state_error("network interface already exists");
    }

    inf_network_interface_esp_netif_impl_entry_t* entry = find_free_entry(ctx);
    if (!entry) {
        return system_failure_error("network interface storage is full");
    }

    esp_netif_inherent_config_t base;
    memcpy(&base, ESP_NETIF_BASE_DEFAULT_ETH, sizeof(esp_netif_inherent_config_t));
    base.if_key = if_key;

    esp_netif_config_t config;
    memset(&config, 0, sizeof(esp_netif_config_t));
    config.base  = &base;
    config.stack = ESP_NETIF_NETSTACK_DEFAULT_ETH;

    esp_netif_t* netif = esp_netif_new(&config);
    if (!netif) {
        return system_failure_error("esp_netif_new failed");
    }

    store_entry(entry, if_key, netif, INF_NETWORK_INTERFACE_ESP_NETIF_IMPL_KIND_ETHERNET);

    return NULL;
}

static void store_entry(inf_network_interface_esp_netif_impl_entry_t* entry, const char* if_key, esp_netif_t* netif, inf_network_interface_esp_netif_impl_kind_t kind) {
    memset(entry, 0, sizeof(inf_network_interface_esp_netif_impl_entry_t));
    entry->used  = true;
    entry->netif = netif;
    entry->kind  = kind;
    copy_cstr(entry->if_key, sizeof(entry->if_key), if_key);
}
