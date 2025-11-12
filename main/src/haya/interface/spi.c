#include "haya/interface/spi.h"

esp_err_t hyInterfaceSPISetup(
    spi_host_device_t spi_host,
    int mosi_pin,
    int miso_pin,
    int sclk_pin)
{
    spi_bus_config_t spi_config = {
        .mosi_io_num = mosi_pin,
        .miso_io_num = miso_pin,
        .sclk_io_num = sclk_pin,
    };

    return spi_bus_initialize(
        spi_host,
        &spi_config,
        SPI_DMA_CH_AUTO);
}