#include "infrastructure/network/interface/stub_impl.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "domain/contracts/network/interface.h"
#include "domain/models/error.h"
#include "domain/models/network.h"

#define STUB_NETWORK_INTERFACE_IMPL_ACTOR "network_interface_stub"

/* Helper Function Prototypes */

static dom_models_error_t*                      bad_argument_error(const char* msg);
static dom_models_error_t*                      bad_state_error(const char* msg);
static dom_models_error_t*                      not_supported_error(const char* msg);
static dom_models_error_t*                      system_failure_error(const char* msg);
static size_t                                   bounded_strlen(const char* value, size_t max_len);
static void                                     copy_cstr(char* dst, size_t dst_size, const char* src);
static void                                     copy_mac(uint8_t dst[DOM_MODELS_NETWORK_MAC_LEN], const uint8_t src[DOM_MODELS_NETWORK_MAC_LEN]);
static void                                     set_ipv4(dom_models_network_ipv4_info_t* out, uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3, uint8_t gw0, uint8_t gw1, uint8_t gw2, uint8_t gw3);
static void                                     set_dns_ipv4(dom_models_network_dns_info_t* out, uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3);
static inf_network_interface_stub_impl_entry_t* find_entry(inf_network_interface_stub_impl_ctx_t* ctx, const char* if_key);
static inf_network_interface_stub_impl_entry_t* find_free_entry(inf_network_interface_stub_impl_ctx_t* ctx);
static inf_network_interface_stub_impl_entry_t* latest_entry(inf_network_interface_stub_impl_ctx_t* ctx);
static void                                     remove_entry(inf_network_interface_stub_impl_ctx_t* ctx, inf_network_interface_stub_impl_entry_t* entry);
static void                                     reset_entries(inf_network_interface_stub_impl_ctx_t* ctx);
static void                                     fill_common(dom_models_network_interface_t* out, const char* if_key, const char* desc, dom_models_network_interface_type_t type, uint32_t flags, int impl_index, int route_prio, bool is_up);
static void                                     fill_wifi_sta(dom_models_network_interface_t* out, const char* if_key);
static void                                     fill_wifi_ap(dom_models_network_interface_t* out, const char* if_key);
static void                                     fill_ethernet(dom_models_network_interface_t* out, const char* if_key);
static dom_models_error_t*                      add_interface(inf_network_interface_stub_impl_ctx_t* ctx, const char* if_key, dom_models_network_interface_type_t type);
static dom_models_network_dhcp_status_t         active_dhcp_status(uint32_t flags, uint32_t dhcp_flag);

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

dom_contracts_network_interface_t* inf_network_interface_stub_impl_new(const inf_network_interface_stub_impl_cfg_t* cfg) {
    inf_network_interface_stub_impl_ctx_t* ctx = (inf_network_interface_stub_impl_ctx_t*)calloc(1, sizeof(inf_network_interface_stub_impl_ctx_t));
    if (!ctx) {
        return NULL;
    }

    if (!cfg) {
        inf_network_interface_stub_impl_cfg_t default_cfg = INF_NETWORK_INTERFACE_STUB_IMPL_CFG_DEFAULT();
        memcpy(&ctx->cfg, &default_cfg, sizeof(inf_network_interface_stub_impl_cfg_t));
    } else {
        memcpy(&ctx->cfg, cfg, sizeof(inf_network_interface_stub_impl_cfg_t));
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

void inf_network_interface_stub_impl_delete(dom_contracts_network_interface_t* self) {
    if (!self || !self->ctx) {
        return;
    }

    inf_network_interface_stub_impl_ctx_t* ctx = self->ctx;

    dom_models_error_t* err = inf_network_interface_stub_impl_deinit(self);
    dom_models_error_delete(err);

    free(ctx);
    dom_contracts_network_interface_delete(self);
}

dom_models_error_t* inf_network_interface_stub_impl_init(dom_contracts_network_interface_t* self) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing network interface stub context");
    }

    inf_network_interface_stub_impl_ctx_t* ctx = self->ctx;
    ctx->initialized                           = true;

    return NULL;
}

dom_models_error_t* inf_network_interface_stub_impl_deinit(dom_contracts_network_interface_t* self) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing network interface stub context");
    }

    inf_network_interface_stub_impl_ctx_t* ctx = self->ctx;

    reset_entries(ctx);
    ctx->initialized = false;

    return NULL;
}

/* Contract Function Implementations */

static dom_models_error_t* create_wifi_sta_impl(
    dom_contracts_network_interface_t* self
) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing network interface stub context");
    }

    inf_network_interface_stub_impl_ctx_t* ctx = self->ctx;

    return add_interface(ctx, ctx->cfg.sta_if_key, DOM_MODELS_NETWORK_INTERFACE_TYPE_WIFI_STA);
}

static dom_models_error_t* create_wifi_ap_impl(
    dom_contracts_network_interface_t* self
) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing network interface stub context");
    }

    inf_network_interface_stub_impl_ctx_t* ctx = self->ctx;

    return add_interface(ctx, ctx->cfg.ap_if_key, DOM_MODELS_NETWORK_INTERFACE_TYPE_WIFI_AP);
}

static dom_models_error_t* create_ethernet_impl(
    dom_contracts_network_interface_t* self
) {
    if (!self || !self->ctx) {
        return bad_argument_error("missing network interface stub context");
    }

    inf_network_interface_stub_impl_ctx_t* ctx = self->ctx;

    return add_interface(ctx, ctx->cfg.eth_if_key, DOM_MODELS_NETWORK_INTERFACE_TYPE_ETHERNET);
}

static dom_models_error_t* attach_ethernet_impl(
    dom_contracts_network_interface_t* self,
    const char*                        if_key,
    void*                              driver_handle
) {
    if (!self || !self->ctx || !if_key || !driver_handle) {
        return bad_argument_error("missing ethernet attach argument");
    }

    inf_network_interface_stub_impl_ctx_t*   ctx   = self->ctx;
    inf_network_interface_stub_impl_entry_t* entry = find_entry(ctx, if_key);
    if (!entry) {
        return bad_argument_error("network interface not found");
    }
    if (entry->data.type != DOM_MODELS_NETWORK_INTERFACE_TYPE_ETHERNET) {
        return bad_argument_error("network interface is not ethernet");
    }

    entry->attached   = true;
    entry->data.is_up = true;

    return NULL;
}

static dom_models_error_t* destroy_impl(
    dom_contracts_network_interface_t* self,
    const char*                        if_key
) {
    if (!self || !self->ctx || !if_key) {
        return bad_argument_error("missing network interface destroy argument");
    }

    inf_network_interface_stub_impl_ctx_t*   ctx   = self->ctx;
    inf_network_interface_stub_impl_entry_t* entry = find_entry(ctx, if_key);
    if (!entry) {
        return bad_argument_error("network interface not found");
    }

    remove_entry(ctx, entry);

    return NULL;
}

static dom_models_error_t* set_hostname_impl(
    dom_contracts_network_interface_t* self,
    const char*                        if_key,
    const char*                        hostname
) {
    if (!self || !self->ctx || !if_key || !hostname) {
        return bad_argument_error("missing network interface hostname argument");
    }

    inf_network_interface_stub_impl_ctx_t*   ctx   = self->ctx;
    inf_network_interface_stub_impl_entry_t* entry = find_entry(ctx, if_key);
    if (!entry) {
        return bad_argument_error("network interface not found");
    }

    entry->data.hostname_available = true;
    copy_cstr(entry->data.hostname, sizeof(entry->data.hostname), hostname);

    return NULL;
}

static dom_models_error_t* start_dhcp_client_impl(
    dom_contracts_network_interface_t* self,
    const char*                        if_key
) {
    if (!self || !self->ctx || !if_key) {
        return bad_argument_error("missing network interface dhcp client argument");
    }

    inf_network_interface_stub_impl_ctx_t*   ctx   = self->ctx;
    inf_network_interface_stub_impl_entry_t* entry = find_entry(ctx, if_key);
    if (!entry) {
        return bad_argument_error("network interface not found");
    }
    if ((entry->data.flags & DOM_MODELS_NETWORK_INTERFACE_FLAG_DHCP_CLIENT) == 0) {
        return not_supported_error("dhcp client is not supported");
    }

    entry->data.dhcp_client_status_available = true;
    entry->data.dhcp_client_status           = DOM_MODELS_NETWORK_DHCP_STATUS_STARTED;

    return NULL;
}

static dom_models_error_t* stop_dhcp_client_impl(
    dom_contracts_network_interface_t* self,
    const char*                        if_key
) {
    if (!self || !self->ctx || !if_key) {
        return bad_argument_error("missing network interface dhcp client argument");
    }

    inf_network_interface_stub_impl_ctx_t*   ctx   = self->ctx;
    inf_network_interface_stub_impl_entry_t* entry = find_entry(ctx, if_key);
    if (!entry) {
        return bad_argument_error("network interface not found");
    }
    if ((entry->data.flags & DOM_MODELS_NETWORK_INTERFACE_FLAG_DHCP_CLIENT) == 0) {
        return not_supported_error("dhcp client is not supported");
    }

    entry->data.dhcp_client_status_available = true;
    entry->data.dhcp_client_status           = DOM_MODELS_NETWORK_DHCP_STATUS_STOPPED;

    return NULL;
}

static dom_models_error_t* start_dhcp_server_impl(
    dom_contracts_network_interface_t* self,
    const char*                        if_key
) {
    if (!self || !self->ctx || !if_key) {
        return bad_argument_error("missing network interface dhcp server argument");
    }

    inf_network_interface_stub_impl_ctx_t*   ctx   = self->ctx;
    inf_network_interface_stub_impl_entry_t* entry = find_entry(ctx, if_key);
    if (!entry) {
        return bad_argument_error("network interface not found");
    }
    if ((entry->data.flags & DOM_MODELS_NETWORK_INTERFACE_FLAG_DHCP_SERVER) == 0) {
        return not_supported_error("dhcp server is not supported");
    }

    entry->data.dhcp_server_status_available = true;
    entry->data.dhcp_server_status           = DOM_MODELS_NETWORK_DHCP_STATUS_STARTED;

    return NULL;
}

static dom_models_error_t* stop_dhcp_server_impl(
    dom_contracts_network_interface_t* self,
    const char*                        if_key
) {
    if (!self || !self->ctx || !if_key) {
        return bad_argument_error("missing network interface dhcp server argument");
    }

    inf_network_interface_stub_impl_ctx_t*   ctx   = self->ctx;
    inf_network_interface_stub_impl_entry_t* entry = find_entry(ctx, if_key);
    if (!entry) {
        return bad_argument_error("network interface not found");
    }
    if ((entry->data.flags & DOM_MODELS_NETWORK_INTERFACE_FLAG_DHCP_SERVER) == 0) {
        return not_supported_error("dhcp server is not supported");
    }

    entry->data.dhcp_server_status_available = true;
    entry->data.dhcp_server_status           = DOM_MODELS_NETWORK_DHCP_STATUS_STOPPED;

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

    inf_network_interface_stub_impl_ctx_t*   ctx   = self->ctx;
    inf_network_interface_stub_impl_entry_t* entry = find_entry(ctx, if_key);
    if (!entry) {
        return bad_argument_error("network interface not found");
    }

    memcpy(out, &entry->data, sizeof(dom_models_network_interface_t));

    return NULL;
}

static dom_models_error_t* get_interfaces_impl(
    dom_contracts_network_interface_t* self,
    dom_models_network_t*              out
) {
    if (!self || !self->ctx || !out) {
        return bad_argument_error("missing network interfaces get argument");
    }

    inf_network_interface_stub_impl_ctx_t* ctx = self->ctx;

    memset(out, 0, sizeof(dom_models_network_t));

    for (size_t i = 0; i < DOM_MODELS_NETWORK_MAX_INTERFACES; i++) {
        if (!ctx->entries[i].used) {
            continue;
        }

        out->total_count++;
        if (out->count < DOM_MODELS_NETWORK_MAX_INTERFACES) {
            memcpy(&out->interfaces[out->count], &ctx->entries[i].data, sizeof(dom_models_network_interface_t));
            out->count++;
        } else {
            out->truncated = true;
        }
    }

    return NULL;
}

/* Helper Function Implementations */

static dom_models_error_t* bad_argument_error(const char* msg) {
    return dom_models_error_new(STUB_NETWORK_INTERFACE_IMPL_ACTOR, DOMAIN_MODELS_ERROR_TYPE_BAD_ARGUMENT, "%s", msg);
}

static dom_models_error_t* bad_state_error(const char* msg) {
    return dom_models_error_new(STUB_NETWORK_INTERFACE_IMPL_ACTOR, DOMAIN_MODELS_ERROR_TYPE_BAD_STATE, "%s", msg);
}

static dom_models_error_t* not_supported_error(const char* msg) {
    return dom_models_error_new(STUB_NETWORK_INTERFACE_IMPL_ACTOR, DOMAIN_MODELS_ERROR_TYPE_NOT_SUPPORTED, "%s", msg);
}

static dom_models_error_t* system_failure_error(const char* msg) {
    return dom_models_error_new(STUB_NETWORK_INTERFACE_IMPL_ACTOR, DOMAIN_MODELS_ERROR_TYPE_SYSTEM_FAILURE, "%s", msg);
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

static void copy_mac(uint8_t dst[DOM_MODELS_NETWORK_MAC_LEN], const uint8_t src[DOM_MODELS_NETWORK_MAC_LEN]) {
    memcpy(dst, src, DOM_MODELS_NETWORK_MAC_LEN);
}

static void set_ipv4(dom_models_network_ipv4_info_t* out, uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3, uint8_t gw0, uint8_t gw1, uint8_t gw2, uint8_t gw3) {
    out->available  = true;
    out->ip[0]      = ip0;
    out->ip[1]      = ip1;
    out->ip[2]      = ip2;
    out->ip[3]      = ip3;
    out->netmask[0] = 255;
    out->netmask[1] = 255;
    out->netmask[2] = 255;
    out->netmask[3] = 0;
    out->gateway[0] = gw0;
    out->gateway[1] = gw1;
    out->gateway[2] = gw2;
    out->gateway[3] = gw3;
}

static void set_dns_ipv4(dom_models_network_dns_info_t* out, uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3) {
    out->available     = true;
    out->addr.family   = DOM_MODELS_NETWORK_IP_FAMILY_IPV4;
    out->addr.bytes[0] = ip0;
    out->addr.bytes[1] = ip1;
    out->addr.bytes[2] = ip2;
    out->addr.bytes[3] = ip3;
}

static inf_network_interface_stub_impl_entry_t* find_entry(inf_network_interface_stub_impl_ctx_t* ctx, const char* if_key) {
    if (!ctx || !if_key) {
        return NULL;
    }

    for (size_t i = 0; i < DOM_MODELS_NETWORK_MAX_INTERFACES; i++) {
        if (ctx->entries[i].used && strncmp(ctx->entries[i].data.if_key, if_key, sizeof(ctx->entries[i].data.if_key)) == 0) {
            return &ctx->entries[i];
        }
    }

    return NULL;
}

static inf_network_interface_stub_impl_entry_t* find_free_entry(inf_network_interface_stub_impl_ctx_t* ctx) {
    for (size_t i = 0; i < DOM_MODELS_NETWORK_MAX_INTERFACES; i++) {
        if (!ctx->entries[i].used) {
            return &ctx->entries[i];
        }
    }

    return NULL;
}

static inf_network_interface_stub_impl_entry_t* latest_entry(inf_network_interface_stub_impl_ctx_t* ctx) {
    for (size_t i = DOM_MODELS_NETWORK_MAX_INTERFACES; i > 0; i--) {
        if (ctx->entries[i - 1].used) {
            return &ctx->entries[i - 1];
        }
    }

    return NULL;
}

static void remove_entry(inf_network_interface_stub_impl_ctx_t* ctx, inf_network_interface_stub_impl_entry_t* entry) {
    inf_network_interface_stub_impl_entry_t* latest = latest_entry(ctx);
    if (!latest || !entry) {
        return;
    }

    if (entry != latest) {
        memcpy(entry, latest, sizeof(inf_network_interface_stub_impl_entry_t));
    }

    memset(latest, 0, sizeof(inf_network_interface_stub_impl_entry_t));
}

static void reset_entries(inf_network_interface_stub_impl_ctx_t* ctx) {
    memset(ctx->entries, 0, sizeof(ctx->entries));
}

static void fill_common(dom_models_network_interface_t* out, const char* if_key, const char* desc, dom_models_network_interface_type_t type, uint32_t flags, int impl_index, int route_prio, bool is_up) {
    memset(out, 0, sizeof(dom_models_network_interface_t));

    copy_cstr(out->if_key, sizeof(out->if_key), if_key);
    copy_cstr(out->desc, sizeof(out->desc), desc);
    copy_cstr(out->impl_name, sizeof(out->impl_name), "stub");

    out->type                         = type;
    out->flags                        = flags;
    out->is_up                        = is_up;
    out->impl_name_available          = true;
    out->impl_index_available         = true;
    out->impl_index                   = impl_index;
    out->route_prio_available         = true;
    out->route_prio                   = route_prio;
    out->mtu_available                = true;
    out->mtu                          = 1500;
    out->dhcp_client_status_available = (flags & DOM_MODELS_NETWORK_INTERFACE_FLAG_DHCP_CLIENT) != 0;
    out->dhcp_client_status           = active_dhcp_status(flags, DOM_MODELS_NETWORK_INTERFACE_FLAG_DHCP_CLIENT);
    out->dhcp_server_status_available = (flags & DOM_MODELS_NETWORK_INTERFACE_FLAG_DHCP_SERVER) != 0;
    out->dhcp_server_status           = active_dhcp_status(flags, DOM_MODELS_NETWORK_INTERFACE_FLAG_DHCP_SERVER);
}

static void fill_wifi_sta(dom_models_network_interface_t* out, const char* if_key) {
    static const uint8_t mac[DOM_MODELS_NETWORK_MAC_LEN] = {0x02, 0x00, 0x00, 0x00, 0x40, 0x01};

    fill_common(
        out,
        if_key,
        "stub wifi station",
        DOM_MODELS_NETWORK_INTERFACE_TYPE_WIFI_STA,
        DOM_MODELS_NETWORK_INTERFACE_FLAG_DHCP_CLIENT |
            DOM_MODELS_NETWORK_INTERFACE_FLAG_GARP |
            DOM_MODELS_NETWORK_INTERFACE_FLAG_EVENT_IP_MODIFIED |
            DOM_MODELS_NETWORK_INTERFACE_FLAG_MLDV6_REPORT |
            DOM_MODELS_NETWORK_INTERFACE_FLAG_IPV6_AUTOCONFIG,
        0,
        100,
        true
    );

    out->mac_available = true;
    copy_mac(out->mac, mac);
    set_ipv4(&out->ipv4, 192, 168, 1, 50, 192, 168, 1, 1);
    set_dns_ipv4(&out->dns[DOM_MODELS_NETWORK_DNS_MAIN], 1, 1, 1, 1);
}

static void fill_wifi_ap(dom_models_network_interface_t* out, const char* if_key) {
    static const uint8_t mac[DOM_MODELS_NETWORK_MAC_LEN] = {0x02, 0x00, 0x00, 0x00, 0x40, 0x02};

    fill_common(
        out,
        if_key,
        "stub wifi access point",
        DOM_MODELS_NETWORK_INTERFACE_TYPE_WIFI_AP,
        DOM_MODELS_NETWORK_INTERFACE_FLAG_DHCP_SERVER |
            DOM_MODELS_NETWORK_INTERFACE_FLAG_AUTO_UP,
        1,
        10,
        true
    );

    out->mac_available = true;
    copy_mac(out->mac, mac);
    set_ipv4(&out->ipv4, 192, 168, 4, 1, 192, 168, 4, 1);
}

static void fill_ethernet(dom_models_network_interface_t* out, const char* if_key) {
    static const uint8_t mac[DOM_MODELS_NETWORK_MAC_LEN] = {0x02, 0x00, 0x00, 0x00, 0x40, 0x03};

    fill_common(
        out,
        if_key,
        "stub ethernet",
        DOM_MODELS_NETWORK_INTERFACE_TYPE_ETHERNET,
        DOM_MODELS_NETWORK_INTERFACE_FLAG_DHCP_CLIENT |
            DOM_MODELS_NETWORK_INTERFACE_FLAG_GARP |
            DOM_MODELS_NETWORK_INTERFACE_FLAG_EVENT_IP_MODIFIED |
            DOM_MODELS_NETWORK_INTERFACE_FLAG_IPV6_AUTOCONFIG,
        2,
        50,
        false
    );

    out->mac_available = true;
    copy_mac(out->mac, mac);
    set_ipv4(&out->ipv4, 192, 168, 2, 50, 192, 168, 2, 1);
    set_dns_ipv4(&out->dns[DOM_MODELS_NETWORK_DNS_MAIN], 1, 1, 1, 1);
}

static dom_models_error_t* add_interface(inf_network_interface_stub_impl_ctx_t* ctx, const char* if_key, dom_models_network_interface_type_t type) {
    size_t if_key_len = bounded_strlen(if_key, DOM_MODELS_NETWORK_IF_KEY_LEN);

    if (if_key_len == 0 || if_key_len >= DOM_MODELS_NETWORK_IF_KEY_LEN) {
        return bad_argument_error("invalid network interface key");
    }
    if (find_entry(ctx, if_key)) {
        return bad_state_error("network interface already exists");
    }

    inf_network_interface_stub_impl_entry_t* entry = find_free_entry(ctx);
    if (!entry) {
        return system_failure_error("network interface storage is full");
    }

    memset(entry, 0, sizeof(inf_network_interface_stub_impl_entry_t));
    entry->used = true;

    switch (type) {
        case DOM_MODELS_NETWORK_INTERFACE_TYPE_WIFI_STA:
            fill_wifi_sta(&entry->data, if_key);
            break;
        case DOM_MODELS_NETWORK_INTERFACE_TYPE_WIFI_AP:
            fill_wifi_ap(&entry->data, if_key);
            break;
        case DOM_MODELS_NETWORK_INTERFACE_TYPE_ETHERNET:
            fill_ethernet(&entry->data, if_key);
            break;
        default:
            memset(entry, 0, sizeof(inf_network_interface_stub_impl_entry_t));
            return bad_argument_error("unsupported network interface type");
    }

    return NULL;
}

static dom_models_network_dhcp_status_t active_dhcp_status(uint32_t flags, uint32_t dhcp_flag) {
    if ((flags & dhcp_flag) == 0) {
        return DOM_MODELS_NETWORK_DHCP_STATUS_UNKNOWN;
    }

    return DOM_MODELS_NETWORK_DHCP_STATUS_STARTED;
}
