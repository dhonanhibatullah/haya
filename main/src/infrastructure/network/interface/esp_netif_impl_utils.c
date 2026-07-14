#include "infrastructure/network/interface/esp_netif_impl_utils.h"

#include <string.h>

#include "domain/models/network.h"
#include "esp_netif.h"

/* Helper Function Prototypes */

static size_t bounded_strlen(const char* value, size_t max_len);
static void copy_cstr(char* out, size_t out_size, const char* value);
static uint32_t flags_to_domain(esp_netif_flags_t flags);
static dom_models_network_dhcp_status_t dhcp_status_to_domain(esp_netif_dhcp_status_t status);
static dom_models_network_interface_type_t infer_type(const char* if_key, const char* desc, uint32_t flags);
static void copy_ip4_addr(uint8_t out[DOM_MODELS_NETWORK_IPV4_LEN], const esp_ip4_addr_t* in);
static void copy_ip4_info(dom_models_network_ipv4_info_t* out, const esp_netif_ip_info_t* in);
static void copy_ip6_addr(dom_models_network_ipv6_addr_t* out, const esp_ip6_addr_t* in);
static void copy_ip_addr(dom_models_network_ip_addr_t* out, const esp_ip_addr_t* in);
static void load_dns(esp_netif_t* netif, dom_models_network_interface_t* out);
static void load_ipv6(esp_netif_t* netif, dom_models_network_interface_t* out);

dom_models_error_t inf_network_interface_esp_netif_impl_error_from_esp(esp_err_t err) {
    switch (err) {
        case ESP_OK:
            return DOMAIN_MODELS_ERROR_OK;
        case ESP_ERR_NO_MEM:
            return DOMAIN_MODELS_ERROR_MALLOC_FAILED;
        case ESP_ERR_INVALID_ARG:
        case ESP_ERR_ESP_NETIF_INVALID_PARAMS:
            return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
        case ESP_ERR_INVALID_STATE:
        case ESP_ERR_ESP_NETIF_DHCP_ALREADY_STARTED:
        case ESP_ERR_ESP_NETIF_DHCP_ALREADY_STOPPED:
        case ESP_ERR_ESP_NETIF_DHCP_NOT_STOPPED:
        case ESP_ERR_ESP_NETIF_IF_NOT_READY:
            return DOMAIN_MODELS_ERROR_BAD_STATE;
        case ESP_ERR_NOT_SUPPORTED:
            return DOMAIN_MODELS_ERROR_NOT_SUPPORTED;
        case ESP_ERR_TIMEOUT:
            return DOMAIN_MODELS_ERROR_TIMEOUT;
        default:
            return DOMAIN_MODELS_ERROR_FAILURE;
    }
}

void inf_network_interface_esp_netif_impl_fill_interface(esp_netif_t* netif, dom_models_network_interface_t* out) {
    if (!netif || !out) {
        return;
    }

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
        out->impl_name[sizeof(out->impl_name) - 1] = '\0';
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

    esp_netif_flags_t esp_flags = esp_netif_get_flags(netif);
    if (esp_flags & ESP_NETIF_DHCP_CLIENT) {
        esp_netif_dhcp_status_t status;
        err = esp_netif_dhcpc_get_status(netif, &status);
        if (err == ESP_OK) {
            out->dhcp_client_status_available = true;
            out->dhcp_client_status           = dhcp_status_to_domain(status);
        }
    }

    if (esp_flags & ESP_NETIF_DHCP_SERVER) {
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

/* Helper Function Implementations */

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

static void copy_cstr(char* out, size_t out_size, const char* value) {
    if (!out || out_size == 0) {
        return;
    }

    size_t len = bounded_strlen(value, out_size - 1);
    if (len > 0) {
        memcpy(out, value, len);
    }
    out[len] = '\0';
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
