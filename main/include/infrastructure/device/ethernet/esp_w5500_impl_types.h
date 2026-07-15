#ifndef INFRASTRUCTURE_DEVICE_ETHERNET_ESP_W5500_IMPL_TYPES_H
#define INFRASTRUCTURE_DEVICE_ETHERNET_ESP_W5500_IMPL_TYPES_H

#include <stdbool.h>
#include <stddef.h>

#include "domain/models/ethernet.h"
#include "esp_eth_driver.h"
#include "esp_eth_netif_glue.h"
#include "esp_event_base.h"
#include "esp_netif_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    esp_eth_handle_t eth_handle;
    const char*      if_key;
    const char*      if_desc;
    int              route_prio;
    bool             attach_netif_glue;
    bool             register_event_handler;
    bool             register_ip_event_handler;
} inf_device_ethernet_esp_w5500_impl_cfg_t;

#define INF_DEVICE_ETHERNET_ESP_W5500_IMPL_EVENT_CALLBACK_MAX 4

#define INF_DEVICE_ETHERNET_ESP_W5500_IMPL_CFG_DEFAULT() \
    {                                                    \
        .eth_handle                = NULL,               \
        .if_key                    = "ETH_DEF",          \
        .if_desc                   = "eth",              \
        .route_prio                = 50,                 \
        .attach_netif_glue         = true,               \
        .register_event_handler    = true,               \
        .register_ip_event_handler = true,               \
    }

typedef struct {
    inf_device_ethernet_esp_w5500_impl_cfg_t cfg;
    esp_netif_t*                             netif;
    esp_eth_netif_glue_handle_t              netif_glue;
    esp_event_handler_instance_t             eth_event_handler;
    esp_event_handler_instance_t             ip_got_event_handler;
    esp_event_handler_instance_t             ip_lost_event_handler;
    bool                                     initialized;
    bool                                     netif_attached;
    bool                                     eth_event_handler_registered;
    bool                                     ip_got_event_handler_registered;
    bool                                     ip_lost_event_handler_registered;
    bool                                     started;
    bool                                     link_up;
    bool                                     promiscuous;
    dom_models_ethernet_event_callback_t     event_cb_funcs[INF_DEVICE_ETHERNET_ESP_W5500_IMPL_EVENT_CALLBACK_MAX];
    void*                                    event_cb_ctxs[INF_DEVICE_ETHERNET_ESP_W5500_IMPL_EVENT_CALLBACK_MAX];
    size_t                                   event_cb_cnt;
} inf_device_ethernet_esp_w5500_impl_ctx_t;

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_DEVICE_ETHERNET_ESP_W5500_IMPL_TYPES_H */
