#include "haya/peripheral/eth.h"

esp_err_t hyPeripheralEthW5500Setup(
    esp_netif_t **netif,
    spi_host_device_t spi_host,
    int spi_clock_speed,
    int cs_pin,
    int int_pin,
    int rst_pin)
{
    spi_device_interface_config_t devcfg = {
        .command_bits = 16,
        .address_bits = 8,
        .mode = 0,
        .clock_speed_hz = spi_clock_speed,
        .spics_io_num = cs_pin,
        .queue_size = 20,
    };

    spi_device_handle_t spi_handle = NULL;
    esp_err_t err = spi_bus_add_device(spi_host, &devcfg, &spi_handle);
    if (err != ESP_OK)
        return err;

    gpio_install_isr_service(0);

    eth_w5500_config_t w5500_cfg = ETH_W5500_DEFAULT_CONFIG(spi_host, &devcfg);
    w5500_cfg.int_gpio_num = int_pin;

    eth_mac_config_t mac_cfg = ETH_MAC_DEFAULT_CONFIG();
    esp_eth_mac_t *mac = esp_eth_mac_new_w5500(&w5500_cfg, &mac_cfg);

    eth_phy_config_t phy_cfg = ETH_PHY_DEFAULT_CONFIG();
    phy_cfg.reset_gpio_num = rst_pin;
    esp_eth_phy_t *phy = esp_eth_phy_new_w5500(&phy_cfg);

    esp_eth_config_t eth_cfg = ETH_DEFAULT_CONFIG(mac, phy);
    esp_eth_handle_t eth_handle = NULL;

    err = esp_eth_driver_install(&eth_cfg, &eth_handle);
    if (err != ESP_OK)
    {
        spi_bus_remove_device(spi_handle);
        return err;
    }

    esp_netif_config_t netif_cfg = ESP_NETIF_DEFAULT_ETH();
    *netif = esp_netif_new(&netif_cfg);
    if (*netif == NULL)
    {
        esp_eth_driver_uninstall(eth_handle);
        spi_bus_remove_device(spi_handle);
        return ESP_FAIL;
    }

    esp_eth_netif_glue_handle_t eth_netif_glue = esp_eth_new_netif_glue(eth_handle);
    if (eth_netif_glue == NULL)
    {
        esp_netif_destroy(*netif);
        esp_eth_driver_uninstall(eth_handle);
        spi_bus_remove_device(spi_handle);
        return ESP_FAIL;
    }

    err = esp_netif_attach(*netif, eth_netif_glue);
    if (err != ESP_OK)
    {
        esp_eth_del_netif_glue(eth_netif_glue);
        esp_netif_destroy(*netif);
        esp_eth_driver_uninstall(eth_handle);
        spi_bus_remove_device(spi_handle);
        return err;
    }

    err = esp_eth_start(eth_handle);
    if (err != ESP_OK)
    {
        esp_eth_del_netif_glue(eth_netif_glue);
        esp_netif_destroy(*netif);
        esp_eth_driver_uninstall(eth_handle);
        spi_bus_remove_device(spi_handle);
        return err;
    }

    return ESP_OK;
}