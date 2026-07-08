#ifndef INFRASTRUCTURE_DEVICE_ETHERNET_ESP_W5500_IMPL_H
#define INFRASTRUCTURE_DEVICE_ETHERNET_ESP_W5500_IMPL_H

#include <stdbool.h>
#include <stdint.h>

#include "domain/contracts/device/ethernet.h"
#include "esp_eth_driver.h"
#include "esp_eth_mac.h"
#include "esp_eth_phy.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char*       if_key;
    spi_host_device_t spi_host;
    int               spi_cs_gpio_num;
    int               int_gpio_num;
    int               reset_gpio_num;
    uint32_t          poll_period_ms;
    int               spi_clock_hz;
    int               spi_queue_size;
    uint8_t           spi_mode;
    bool              register_event_handler;
} inf_device_ethernet_esp_w5500_impl_cfg_t;

#define INF_DEVICE_ETHERNET_ESP_W5500_IMPL_CFG_DEFAULT() \
    {                                                    \
        .if_key                 = "ETH_W5500_DEF",       \
        .spi_host               = SPI2_HOST,             \
        .spi_cs_gpio_num        = 5,                     \
        .int_gpio_num           = 4,                     \
        .reset_gpio_num         = -1,                    \
        .poll_period_ms         = 0,                     \
        .spi_clock_hz           = 20 * 1000 * 1000,      \
        .spi_queue_size         = 20,                    \
        .spi_mode               = 0,                     \
        .register_event_handler = true,                  \
    }

typedef struct {
    inf_device_ethernet_esp_w5500_impl_cfg_t cfg;
    esp_eth_mac_t*                           mac;
    esp_eth_phy_t*                           phy;
    esp_eth_handle_t                         eth_handle;
    esp_event_handler_instance_t             eth_event_handler;
    bool                                     eth_initialized;
    bool                                     eth_event_handler_registered;
    bool                                     started;
    bool                                     link_up;
    bool                                     promiscuous;
    bool                                     flow_control;
    bool                                     phy_loopback;
} inf_device_ethernet_esp_w5500_impl_ctx_t;

dom_contracts_device_ethernet_t* inf_device_ethernet_esp_w5500_impl_new(const inf_device_ethernet_esp_w5500_impl_cfg_t* cfg);

void inf_device_ethernet_esp_w5500_impl_delete(dom_contracts_device_ethernet_t* self);

dom_models_error_t* inf_device_ethernet_esp_w5500_impl_init(dom_contracts_device_ethernet_t* self);

dom_models_error_t* inf_device_ethernet_esp_w5500_impl_deinit(dom_contracts_device_ethernet_t* self);

esp_eth_handle_t inf_device_ethernet_esp_w5500_impl_get_handle(dom_contracts_device_ethernet_t* self);

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_DEVICE_ETHERNET_ESP_W5500_IMPL_H */
