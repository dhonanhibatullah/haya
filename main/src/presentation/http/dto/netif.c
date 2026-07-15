#include "presentation/http/dto/netif.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "cJSON.h"
#include "domain/models/network.h"

/* Helper Function Prototypes */

static void mac_to_string(const uint8_t mac[DOM_MODELS_NETWORK_MAC_LEN], char out[18]);
static void ipv4_to_string(const uint8_t value[DOM_MODELS_NETWORK_IPV4_LEN], char out[16]);
static void ipv6_to_string(const uint8_t value[DOM_MODELS_NETWORK_IPV6_LEN], char out[40]);
static const char* network_interface_type_to_string(dom_models_network_interface_type_t type);
static const char* network_dhcp_status_to_string(dom_models_network_dhcp_status_t status);
static cJSON* ipv4_info_to_json(const dom_models_network_ipv4_info_t* info);
static cJSON* ipv6_addr_to_json(const dom_models_network_ipv6_addr_t* info);
static cJSON* dns_info_to_json(const dom_models_network_dns_info_t* info);

cJSON* pres_http_dto_netif_network_to_json(const dom_models_network_t* network) {
    if (!network) {
        return NULL;
    }

    cJSON* root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    cJSON_AddNumberToObject(root, "total_count", (double)network->total_count);
    cJSON_AddNumberToObject(root, "count", (double)network->count);
    cJSON_AddBoolToObject(root, "truncated", network->truncated);

    cJSON* interfaces = cJSON_AddArrayToObject(root, "interfaces");
    if (interfaces) {
        for (size_t i = 0; i < network->count; i++) {
            cJSON_AddItemToArray(interfaces, pres_http_dto_netif_interface_to_json(&network->interfaces[i]));
        }
    }

    return root;
}

cJSON* pres_http_dto_netif_interface_to_json(const dom_models_network_interface_t* interface) {
    if (!interface) {
        return NULL;
    }

    char mac[18];
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, "if_key", interface->if_key);
    cJSON_AddStringToObject(root, "desc", interface->desc);
    cJSON_AddBoolToObject(root, "hostname_available", interface->hostname_available);
    if (interface->hostname_available) {
        cJSON_AddStringToObject(root, "hostname", interface->hostname);
    }
    cJSON_AddBoolToObject(root, "impl_name_available", interface->impl_name_available);
    if (interface->impl_name_available) {
        cJSON_AddStringToObject(root, "impl_name", interface->impl_name);
    }
    cJSON_AddStringToObject(root, "type", network_interface_type_to_string(interface->type));
    cJSON_AddNumberToObject(root, "type_code", (double)interface->type);
    cJSON_AddNumberToObject(root, "flags", (double)interface->flags);
    cJSON_AddBoolToObject(root, "is_default", interface->is_default);
    cJSON_AddBoolToObject(root, "is_up", interface->is_up);
    cJSON_AddBoolToObject(root, "mac_available", interface->mac_available);
    if (interface->mac_available) {
        mac_to_string(interface->mac, mac);
        cJSON_AddStringToObject(root, "mac", mac);
    }
    cJSON_AddBoolToObject(root, "impl_index_available", interface->impl_index_available);
    if (interface->impl_index_available) {
        cJSON_AddNumberToObject(root, "impl_index", interface->impl_index);
    }
    cJSON_AddBoolToObject(root, "route_prio_available", interface->route_prio_available);
    if (interface->route_prio_available) {
        cJSON_AddNumberToObject(root, "route_prio", interface->route_prio);
    }
    cJSON_AddBoolToObject(root, "mtu_available", interface->mtu_available);
    if (interface->mtu_available) {
        cJSON_AddNumberToObject(root, "mtu", interface->mtu);
    }
    cJSON_AddBoolToObject(root, "dhcp_client_status_available", interface->dhcp_client_status_available);
    if (interface->dhcp_client_status_available) {
        cJSON_AddStringToObject(root, "dhcp_client_status", network_dhcp_status_to_string(interface->dhcp_client_status));
    }
    cJSON_AddBoolToObject(root, "dhcp_server_status_available", interface->dhcp_server_status_available);
    if (interface->dhcp_server_status_available) {
        cJSON_AddStringToObject(root, "dhcp_server_status", network_dhcp_status_to_string(interface->dhcp_server_status));
    }
    cJSON_AddItemToObject(root, "ipv4", ipv4_info_to_json(&interface->ipv4));
    cJSON_AddItemToObject(root, "old_ipv4", ipv4_info_to_json(&interface->old_ipv4));
    cJSON_AddItemToObject(root, "ipv6_linklocal", ipv6_addr_to_json(&interface->ipv6_linklocal));
    cJSON_AddItemToObject(root, "ipv6_global", ipv6_addr_to_json(&interface->ipv6_global));

    cJSON* dns = cJSON_AddArrayToObject(root, "dns");
    if (dns) {
        for (size_t i = 0; i < DOM_MODELS_NETWORK_DNS_MAX; i++) {
            cJSON_AddItemToArray(dns, dns_info_to_json(&interface->dns[i]));
        }
    }

    return root;
}

/* Helper Function Implementations */

static void mac_to_string(const uint8_t mac[DOM_MODELS_NETWORK_MAC_LEN], char out[18]) {
    snprintf(
        out,
        18,
        "%02X:%02X:%02X:%02X:%02X:%02X",
        mac[0],
        mac[1],
        mac[2],
        mac[3],
        mac[4],
        mac[5]
    );
}

static void ipv4_to_string(const uint8_t value[DOM_MODELS_NETWORK_IPV4_LEN], char out[16]) {
    snprintf(out, 16, "%u.%u.%u.%u", value[0], value[1], value[2], value[3]);
}

static void ipv6_to_string(const uint8_t value[DOM_MODELS_NETWORK_IPV6_LEN], char out[40]) {
    snprintf(
        out,
        40,
        "%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X",
        value[0],
        value[1],
        value[2],
        value[3],
        value[4],
        value[5],
        value[6],
        value[7],
        value[8],
        value[9],
        value[10],
        value[11],
        value[12],
        value[13],
        value[14],
        value[15]
    );
}

static const char* network_interface_type_to_string(dom_models_network_interface_type_t type) {
    switch (type) {
        case DOM_MODELS_NETWORK_INTERFACE_TYPE_WIFI_STA:
            return "WIFI_STA";
        case DOM_MODELS_NETWORK_INTERFACE_TYPE_WIFI_AP:
            return "WIFI_AP";
        case DOM_MODELS_NETWORK_INTERFACE_TYPE_ETHERNET:
            return "ETHERNET";
        case DOM_MODELS_NETWORK_INTERFACE_TYPE_PPP:
            return "PPP";
        case DOM_MODELS_NETWORK_INTERFACE_TYPE_BRIDGE:
            return "BRIDGE";
        case DOM_MODELS_NETWORK_INTERFACE_TYPE_LOOPBACK:
            return "LOOPBACK";
        case DOM_MODELS_NETWORK_INTERFACE_TYPE_UNKNOWN:
        default:
            return "UNKNOWN";
    }
}

static const char* network_dhcp_status_to_string(dom_models_network_dhcp_status_t status) {
    switch (status) {
        case DOM_MODELS_NETWORK_DHCP_STATUS_INIT:
            return "INIT";
        case DOM_MODELS_NETWORK_DHCP_STATUS_STARTED:
            return "STARTED";
        case DOM_MODELS_NETWORK_DHCP_STATUS_STOPPED:
            return "STOPPED";
        case DOM_MODELS_NETWORK_DHCP_STATUS_UNKNOWN:
        default:
            return "UNKNOWN";
    }
}

static cJSON* ipv4_info_to_json(const dom_models_network_ipv4_info_t* info) {
    char value[16];
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    cJSON_AddBoolToObject(root, "available", info->available);
    if (info->available) {
        ipv4_to_string(info->ip, value);
        cJSON_AddStringToObject(root, "ip", value);
        ipv4_to_string(info->netmask, value);
        cJSON_AddStringToObject(root, "netmask", value);
        ipv4_to_string(info->gateway, value);
        cJSON_AddStringToObject(root, "gateway", value);
    }

    return root;
}

static cJSON* ipv6_addr_to_json(const dom_models_network_ipv6_addr_t* info) {
    char value[40];
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    cJSON_AddBoolToObject(root, "available", info->available);
    if (info->available) {
        ipv6_to_string(info->addr, value);
        cJSON_AddStringToObject(root, "addr", value);
        cJSON_AddNumberToObject(root, "zone", info->zone);
    }

    return root;
}

static cJSON* dns_info_to_json(const dom_models_network_dns_info_t* info) {
    char value[40];
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    cJSON_AddBoolToObject(root, "available", info->available);
    if (info->available) {
        cJSON_AddNumberToObject(root, "family", (double)info->addr.family);
        cJSON_AddNumberToObject(root, "zone", info->addr.zone);
        if (info->addr.family == DOM_MODELS_NETWORK_IP_FAMILY_IPV4) {
            ipv4_to_string(info->addr.bytes, value);
            cJSON_AddStringToObject(root, "addr", value);
        } else if (info->addr.family == DOM_MODELS_NETWORK_IP_FAMILY_IPV6) {
            ipv6_to_string(info->addr.bytes, value);
            cJSON_AddStringToObject(root, "addr", value);
        }
    }

    return root;
}
