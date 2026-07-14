#ifndef INFRASTRUCTURE_NETWORK_INTERFACE_ESP_NETIF_IMPL_TYPES_H
#define INFRASTRUCTURE_NETWORK_INTERFACE_ESP_NETIF_IMPL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* sta_if_key;
} inf_network_interface_esp_netif_impl_cfg_t;

#define INF_NETWORK_INTERFACE_ESP_NETIF_IMPL_CFG_DEFAULT() \
    {                                                      \
        .sta_if_key = "WIFI_STA_DEF",                      \
    }

typedef struct {
    inf_network_interface_esp_netif_impl_cfg_t cfg;
} inf_network_interface_esp_netif_impl_ctx_t;

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_NETWORK_INTERFACE_ESP_NETIF_IMPL_TYPES_H */
