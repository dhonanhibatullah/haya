#ifndef __HAYA_INTERFACE_SPI_H
#define __HAYA_INTERFACE_SPI_H

#include "esp_err.h"
#include "driver/spi_master.h"

esp_err_t hyInterfaceSPISetup(
    spi_host_device_t spi_host,
    int mosi_pin,
    int miso_pin,
    int sclk_pin);

#endif