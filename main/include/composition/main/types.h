#ifndef COMPOSITION_MAIN_TYPES_H
#define COMPOSITION_MAIN_TYPES_H

#include "config.h"            // IWYU pragma: keep
#include "driver/i2c_types.h"  // IWYU pragma: keep
#include "esp_eth_driver.h"
#include "esp_eth_mac.h"      // IWYU pragma: keep
#include "esp_eth_phy.h"      // IWYU pragma: keep
#include "esp_http_server.h"  // IWYU pragma: keep
#include "mqtt_client.h"      // IWYU pragma: keep
#include "nvs.h"              // IWYU pragma: keep
#include "sdmmc_cmd.h"        // IWYU pragma: keep

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_I2C_0_ENABLE
    i2c_master_bus_handle_t i2c_0_bus_handle;
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_I2C_0_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_I2C_1_ENABLE
    i2c_master_bus_handle_t i2c_1_bus_handle;
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_I2C_1_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_NVS_ENABLE
    nvs_handle_t nvs_handle;
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_NVS_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_SD_CARD_ENABLE
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_SD_CARD_USE_SDSPI
    sdmmc_card_t* sd_card_sdspi_card;
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_SD_CARD_USE_SDSPI */
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_SD_CARD_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_ETHERNET_ENABLE
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_ETHERNET_USE_W5500
    esp_eth_handle_t ethernet_w5500_handle;
    esp_eth_mac_t*   ethernet_w5500_mac;
    esp_eth_phy_t*   ethernet_w5500_phy;
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_ETHERNET_USE_W5500 */
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_ETHERNET_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_HTTP_SERVER_ENABLE
    httpd_handle_t http_server_handle;
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_HTTP_SERVER_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_MQTT_CLIENT_ENABLE
    esp_mqtt_client_handle_t mqtt_client_handle;
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_MQTT_CLIENT_ENABLE */
} cmp_main_driver_t;

typedef struct {
} cmp_main_infrastructure_t;

typedef struct {
} cmp_main_application_t;

typedef struct {
} cmp_main_presentation_t;

typedef struct {
    cmp_main_driver_t         driver;
    cmp_main_infrastructure_t infrastructure;
    cmp_main_application_t    application;
    cmp_main_presentation_t   presentation;
} cmp_main_launcher_t;

#ifdef __cplusplus
}
#endif

#endif /* COMPOSITION_MAIN_TYPES_H */
