#ifndef COMPOSITION_MAIN_CONFIG_H
#define COMPOSITION_MAIN_CONFIG_H

#include <stdbool.h>  // IWYU pragma: keep
#include <stddef.h>   // IWYU pragma: keep
#include <stdint.h>   // IWYU pragma: keep

#include "hal/spi_types.h"  // IWYU pragma: keep
#include "soc/gpio_num.h"   // IWYU pragma: keep

/* Driver Config Defines */

#define COMPOSITION_MAIN_CONFIG_DRIVER_ISR_ENABLE
#define COMPOSITION_MAIN_CONFIG_DRIVER_I2C_0_ENABLE
#define COMPOSITION_MAIN_CONFIG_DRIVER_I2C_1_ENABLE
#define COMPOSITION_MAIN_CONFIG_DRIVER_SPI_2_ENABLE
#define COMPOSITION_MAIN_CONFIG_DRIVER_SPI_3_ENABLE
#define COMPOSITION_MAIN_CONFIG_DRIVER_NVS_ENABLE
#define COMPOSITION_MAIN_CONFIG_DRIVER_LITTLEFS_ENABLE
#define COMPOSITION_MAIN_CONFIG_DRIVER_SD_CARD_ENABLE
#define COMPOSITION_MAIN_CONFIG_DRIVER_SD_CARD_USE_SDSPI
#define COMPOSITION_MAIN_CONFIG_DRIVER_EVENT_LOOP_ENABLE
#define COMPOSITION_MAIN_CONFIG_DRIVER_NETIF_ENABLE
#define COMPOSITION_MAIN_CONFIG_DRIVER_WIFI_ENABLE
#define COMPOSITION_MAIN_CONFIG_DRIVER_ETHERNET_ENABLE
#define COMPOSITION_MAIN_CONFIG_DRIVER_ETHERNET_USE_W5500
#define COMPOSITION_MAIN_CONFIG_DRIVER_BLE_ENABLE
#define COMPOSITION_MAIN_CONFIG_DRIVER_HTTP_SERVER_ENABLE
#define COMPOSITION_MAIN_CONFIG_DRIVER_MQTT_CLIENT_ENABLE

/* Infrastructure Config Defines */

/* Application Config Defines */

/* Presentation Config Defines */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    struct driver {
        /* ISR */
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_ISR_ENABLE
        const int isr_intr_alloc_flag;
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_ISR_ENABLE */

        /* I2C 0 */
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_I2C_0_ENABLE
        const gpio_num_t i2c_0_sda_pin;
        const gpio_num_t i2c_0_scl_pin;
        const bool       i2c_0_enable_internal_pullup;
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_I2C_0_ENABLE */

        /* I2C 1 */
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_I2C_1_ENABLE
        const gpio_num_t i2c_1_sda_pin;
        const gpio_num_t i2c_1_scl_pin;
        const bool       i2c_1_enable_internal_pullup;
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_I2C_1_ENABLE */

        /* SPI 2 */
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_SPI_2_ENABLE
        const gpio_num_t spi_2_miso_pin;
        const gpio_num_t spi_2_mosi_pin;
        const gpio_num_t spi_2_sclk_pin;
        const gpio_num_t spi_2_quadwp_pin;
        const gpio_num_t spi_2_quadhd_pin;
        const int        spi_2_max_transfer_size;
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_SPI_2_ENABLE */

        /* SPI 3 */
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_SPI_3_ENABLE
        const gpio_num_t spi_3_miso_pin;
        const gpio_num_t spi_3_mosi_pin;
        const gpio_num_t spi_3_sclk_pin;
        const gpio_num_t spi_3_quadwp_pin;
        const gpio_num_t spi_3_quadhd_pin;
        const int        spi_3_max_transfer_size;
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_SPI_3_ENABLE */

        /* NVS */
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_NVS_ENABLE
        const char* nvs_namespace;
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_NVS_ENABLE */

        /* LittleFS */
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_LITTLEFS_ENABLE
        const char*   littlefs_base_path;
        const char*   littlefs_partition_label;
        const uint8_t littlefs_format_if_failed;
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_LITTLEFS_ENABLE */

        /* SD Card */
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_SD_CARD_ENABLE
        const char*  sd_card_base_path;
        const bool   sd_card_format_if_mount_failed;
        const int    sd_card_max_files;
        const size_t sd_card_allocation_unit_size;
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_SD_CARD_USE_SDSPI
        const gpio_num_t        sd_card_sdspi_cs_pin;
        const spi_host_device_t sd_card_sdspi_spi_host;
        const int               sd_card_sdspi_max_freq_khz;
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_SD_CARD_USE_SDSPI */
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_SD_CARD_ENABLE */

        /* Ethernet */
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_ETHERNET_ENABLE
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_ETHERNET_USE_W5500
        const gpio_num_t        ethernet_w5500_cs_pin;
        const gpio_num_t        ethernet_w5500_int_pin;
        const gpio_num_t        ethernet_w5500_rst_pin;
        const spi_host_device_t ethernet_w5500_spi_host;
        const uint8_t           ethernet_w5500_spi_mode;
        const int               ethernet_w5500_spi_clock_hz;
        const int               ethernet_w5500_spi_queue_size;
        const uint32_t          ethernet_w5500_poll_period_ms;
        const uint32_t          ethernet_w5500_rx_task_stack_size;
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_ETHERNET_USE_W5500 */
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_ETHERNET_ENABLE */

        /* BLE */
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_BLE_ENABLE
        const char* ble_device_name;
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_BLE_ENABLE */
    } driver;

    struct infrastructure {
    } infrastructure;

    struct application {
    } application;

    struct presentation {
    } presentation;

} cmp_main_config_t;

extern const cmp_main_config_t cmp_main_config;

#ifdef __cplusplus
}
#endif

#endif /* COMPOSITION_MAIN_CONFIG_H */
