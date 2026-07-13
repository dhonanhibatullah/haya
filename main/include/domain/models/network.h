#ifndef DOMAIN_MODELS_NETWORK_H
#define DOMAIN_MODELS_NETWORK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DOM_MODELS_NETWORK_MAX_INTERFACES            8
#define DOM_MODELS_NETWORK_IF_KEY_LEN                32
#define DOM_MODELS_NETWORK_DESC_LEN                  64
#define DOM_MODELS_NETWORK_HOSTNAME_LEN              33
#define DOM_MODELS_NETWORK_IMPL_NAME_LEN             8
#define DOM_MODELS_NETWORK_MAC_LEN                   6
#define DOM_MODELS_NETWORK_DNS_MAX                   3
#define DOM_MODELS_NETWORK_IPV4_LEN                  4
#define DOM_MODELS_NETWORK_IPV6_LEN                  16
#define DOM_MODELS_NETWORK_IPV6_MAX                  8

#define DOM_MODELS_NETWORK_INTERFACE_FLAG_DHCP_CLIENT            (1u << 0)
#define DOM_MODELS_NETWORK_INTERFACE_FLAG_DHCP_SERVER            (1u << 1)
#define DOM_MODELS_NETWORK_INTERFACE_FLAG_AUTO_UP                (1u << 2)
#define DOM_MODELS_NETWORK_INTERFACE_FLAG_GARP                   (1u << 3)
#define DOM_MODELS_NETWORK_INTERFACE_FLAG_EVENT_IP_MODIFIED      (1u << 4)
#define DOM_MODELS_NETWORK_INTERFACE_FLAG_PPP                    (1u << 5)
#define DOM_MODELS_NETWORK_INTERFACE_FLAG_BRIDGE                 (1u << 6)
#define DOM_MODELS_NETWORK_INTERFACE_FLAG_MLDV6_REPORT           (1u << 7)
#define DOM_MODELS_NETWORK_INTERFACE_FLAG_IPV6_AUTOCONFIG        (1u << 8)

typedef enum {
    DOM_MODELS_NETWORK_INTERFACE_TYPE_UNKNOWN = 0,
    DOM_MODELS_NETWORK_INTERFACE_TYPE_WIFI_STA,
    DOM_MODELS_NETWORK_INTERFACE_TYPE_WIFI_AP,
    DOM_MODELS_NETWORK_INTERFACE_TYPE_ETHERNET,
    DOM_MODELS_NETWORK_INTERFACE_TYPE_PPP,
    DOM_MODELS_NETWORK_INTERFACE_TYPE_BRIDGE,
    DOM_MODELS_NETWORK_INTERFACE_TYPE_LOOPBACK,
} dom_models_network_interface_type_t;

typedef enum {
    DOM_MODELS_NETWORK_DHCP_STATUS_UNKNOWN = 0,
    DOM_MODELS_NETWORK_DHCP_STATUS_INIT,
    DOM_MODELS_NETWORK_DHCP_STATUS_STARTED,
    DOM_MODELS_NETWORK_DHCP_STATUS_STOPPED,
} dom_models_network_dhcp_status_t;

typedef enum {
    DOM_MODELS_NETWORK_DNS_MAIN = 0,
    DOM_MODELS_NETWORK_DNS_BACKUP,
    DOM_MODELS_NETWORK_DNS_FALLBACK,
} dom_models_network_dns_type_t;

typedef enum {
    DOM_MODELS_NETWORK_IP_FAMILY_NONE = 0,
    DOM_MODELS_NETWORK_IP_FAMILY_IPV4,
    DOM_MODELS_NETWORK_IP_FAMILY_IPV6,
} dom_models_network_ip_family_t;

typedef struct {
    dom_models_network_ip_family_t family;
    uint8_t                        bytes[DOM_MODELS_NETWORK_IPV6_LEN];
    uint8_t                        zone;
} dom_models_network_ip_addr_t;

typedef struct {
    bool    available;
    uint8_t ip[DOM_MODELS_NETWORK_IPV4_LEN];
    uint8_t netmask[DOM_MODELS_NETWORK_IPV4_LEN];
    uint8_t gateway[DOM_MODELS_NETWORK_IPV4_LEN];
} dom_models_network_ipv4_info_t;

typedef struct {
    bool                         available;
    dom_models_network_ip_addr_t addr;
} dom_models_network_dns_info_t;

typedef struct {
    bool    available;
    uint8_t addr[DOM_MODELS_NETWORK_IPV6_LEN];
    uint8_t zone;
} dom_models_network_ipv6_addr_t;

typedef struct {
    char                                    if_key[DOM_MODELS_NETWORK_IF_KEY_LEN];
    char                                    desc[DOM_MODELS_NETWORK_DESC_LEN];
    bool                                    hostname_available;
    char                                    hostname[DOM_MODELS_NETWORK_HOSTNAME_LEN];
    bool                                    impl_name_available;
    char                                    impl_name[DOM_MODELS_NETWORK_IMPL_NAME_LEN];
    dom_models_network_interface_type_t     type;
    uint32_t                                flags;
    bool                                    is_default;
    bool                                    is_up;

    bool                                    mac_available;
    uint8_t                                 mac[DOM_MODELS_NETWORK_MAC_LEN];

    bool                                    impl_index_available;
    int                                     impl_index;
    bool                                    route_prio_available;
    int                                     route_prio;
    bool                                    mtu_available;
    uint16_t                                mtu;

    bool                                    got_ip_event_id_available;
    int32_t                                 got_ip_event_id;
    bool                                    lost_ip_event_id_available;
    int32_t                                 lost_ip_event_id;

    bool                                    dhcp_client_status_available;
    dom_models_network_dhcp_status_t        dhcp_client_status;
    bool                                    dhcp_server_status_available;
    dom_models_network_dhcp_status_t        dhcp_server_status;

    dom_models_network_ipv4_info_t          ipv4;
    dom_models_network_ipv4_info_t          old_ipv4;
    dom_models_network_dns_info_t           dns[DOM_MODELS_NETWORK_DNS_MAX];

    dom_models_network_ipv6_addr_t          ipv6_linklocal;
    dom_models_network_ipv6_addr_t          ipv6_global;
    size_t                                  ipv6_count;
    dom_models_network_ipv6_addr_t          ipv6[DOM_MODELS_NETWORK_IPV6_MAX];
    size_t                                  preferred_ipv6_count;
    dom_models_network_ipv6_addr_t          preferred_ipv6[DOM_MODELS_NETWORK_IPV6_MAX];

} dom_models_network_interface_t;

typedef struct {
    size_t                         total_count;
    size_t                         count;
    bool                           truncated;
    dom_models_network_interface_t interfaces[DOM_MODELS_NETWORK_MAX_INTERFACES];
} dom_models_network_t;

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_MODELS_NETWORK_H */
