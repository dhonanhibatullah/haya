#ifndef __HAYA_PERIPHERAL_ETH_H
#define __HAYA_PERIPHERAL_ETH_H

#include "driver/spi_common.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_eth_mac_w5500.h"
#include "esp_eth_phy_w5500.h"

esp_err_t hyPeripheralEthW5500Setup(
    esp_netif_t **netif,
    spi_host_device_t spi_host,
    int spi_clock_speed,
    int cs_pin,
    int int_pin,
    int rst_pin);

#endif