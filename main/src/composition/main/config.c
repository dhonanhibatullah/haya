#include "composition/main/config.h"

#include "application/wifiman/impl_types.h"                 // IWYU pragma: keep
#include "hal/gpio_types.h"                                 // IWYU pragma: keep
#include "hal/spi_types.h"                                  // IWYU pragma: keep
#include "presentation/task/wifiman_sta_reconnect/types.h"  // IWYU pragma: keep
#include "soc/gpio_num.h"                                   // IWYU pragma: keep

#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_GPIO_ENABLE
static const cmp_main_config_driver_gpio_t cmp_main_gpio_configs[] = {
    {
        .gpio_num             = GPIO_NUM_32,
        .mode                 = GPIO_MODE_INPUT,
        .pull_up_en           = GPIO_PULLUP_ENABLE,
        .pull_down_en         = GPIO_PULLDOWN_DISABLE,
        .intr_type            = GPIO_INTR_DISABLE,
        .initial_output_level = false,
    },
    {
        .gpio_num             = GPIO_NUM_33,
        .mode                 = GPIO_MODE_OUTPUT,
        .pull_up_en           = GPIO_PULLUP_DISABLE,
        .pull_down_en         = GPIO_PULLDOWN_DISABLE,
        .intr_type            = GPIO_INTR_DISABLE,
        .initial_output_level = false,
    },
};
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_GPIO_ENABLE */

const cmp_main_config_t cmp_main_config = {
    .driver = {
/* ISR */
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_ISR_ENABLE
        .isr_intr_alloc_flag = 0,
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_ISR_ENABLE */

/* GPIO */
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_GPIO_ENABLE
        .gpio_configs     = cmp_main_gpio_configs,
        .gpio_configs_cnt = sizeof(cmp_main_gpio_configs) / sizeof(cmp_main_config_driver_gpio_t),
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_GPIO_ENABLE */

/* I2C 0 */
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_I2C_0_ENABLE
        .i2c_0_sda_pin                = GPIO_NUM_21,
        .i2c_0_scl_pin                = GPIO_NUM_22,
        .i2c_0_enable_internal_pullup = true,
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_I2C_0_ENABLE */

/* I2C 1 */
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_I2C_1_ENABLE
        .i2c_1_sda_pin                = GPIO_NUM_21,
        .i2c_1_scl_pin                = GPIO_NUM_22,
        .i2c_1_enable_internal_pullup = true,
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_I2C_1_ENABLE */

/* SPI 2 */
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_SPI_2_ENABLE
        .spi_2_miso_pin          = GPIO_NUM_12,
        .spi_2_mosi_pin          = GPIO_NUM_13,
        .spi_2_sclk_pin          = GPIO_NUM_14,
        .spi_2_quadwp_pin        = GPIO_NUM_NC,
        .spi_2_quadhd_pin        = GPIO_NUM_NC,
        .spi_2_max_transfer_size = 0,
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_SPI_2_ENABLE */

/* SPI 3 */
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_SPI_3_ENABLE
        .spi_3_miso_pin          = GPIO_NUM_19,
        .spi_3_mosi_pin          = GPIO_NUM_23,
        .spi_3_sclk_pin          = GPIO_NUM_18,
        .spi_3_quadwp_pin        = GPIO_NUM_NC,
        .spi_3_quadhd_pin        = GPIO_NUM_NC,
        .spi_3_max_transfer_size = 0,
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_SPI_3_ENABLE */

/* NVS */
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_NVS_ENABLE
        .nvs_namespace = PROJECT_NAME,
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_NVS_ENABLE */

/* LittleFS */
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_LITTLEFS_ENABLE
        .littlefs_base_path        = "/littlefs",
        .littlefs_partition_label  = "storage",
        .littlefs_format_if_failed = 1,
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_LITTLEFS_ENABLE */

/* SD Card */
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_SD_CARD_ENABLE
        .sd_card_base_path              = "/sdcard",
        .sd_card_format_if_mount_failed = false,
        .sd_card_max_files              = 5,
        .sd_card_allocation_unit_size   = 0,
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_SD_CARD_USE_SDSPI
        .sd_card_sdspi_cs_pin       = GPIO_NUM_15,
        .sd_card_sdspi_spi_host     = SPI3_HOST,
        .sd_card_sdspi_max_freq_khz = 20 * 1000,
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_SD_CARD_USE_SDSPI */
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_SD_CARD_ENABLE */

/* Ethernet */
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_ETHERNET_ENABLE
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_ETHERNET_USE_W5500
        .ethernet_w5500_cs_pin             = GPIO_NUM_5,
        .ethernet_w5500_int_pin            = GPIO_NUM_4,
        .ethernet_w5500_rst_pin            = GPIO_NUM_2,
        .ethernet_w5500_spi_host           = SPI3_HOST,
        .ethernet_w5500_spi_mode           = 0,
        .ethernet_w5500_spi_clock_hz       = 20 * 1000 * 1000,
        .ethernet_w5500_spi_queue_size     = 20,
        .ethernet_w5500_poll_period_ms     = 0,
        .ethernet_w5500_rx_task_stack_size = 4096,
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_ETHERNET_USE_W5500 */
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_ETHERNET_ENABLE */

/* BLE */
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_BLE_ENABLE
        .ble_device_name = "haya",
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_BLE_ENABLE */
    },
    .infrastructure = {
#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_LOGGER_LEVELED_STDIO_ENABLE
        .logger_leveled_stdio_level              = DOMAIN_MODELS_LOGGER_LEVEL_INFO,
        .logger_leveled_stdio_callback_max_count = 0,
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_LOGGER_LEVELED_STDIO_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_WIFI_ENABLE
#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_WIFI_USE_ESP_WIFI
        .wifi_esp_wifi_sta_if_key             = "WIFI_STA_DEF",
        .wifi_esp_wifi_ap_if_key              = "WIFI_AP_DEF",
        .wifi_esp_wifi_register_event_handler = true,
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_WIFI_USE_ESP_WIFI */
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_WIFI_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_NETWORK_INTERFACE_ENABLE
#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_NETWORK_INTERFACE_USE_ESP_NETIF
        .network_interface_esp_netif_sta_if_key = "WIFI_STA_DEF",
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_NETWORK_INTERFACE_USE_ESP_NETIF */
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_NETWORK_INTERFACE_ENABLE */
    },
    .application = {
#ifdef COMPOSITION_MAIN_CONFIG_APPLICATION_WIFIMAN_ENABLE
        .wifiman_reconnect_max_trials   = APP_WIFIMAN_IMPL_DEFAULT_RECONNECT_MAX_TRIALS,
        .wifiman_ap_auto_manage_enabled = true,
#endif /* COMPOSITION_MAIN_CONFIG_APPLICATION_WIFIMAN_ENABLE */
    },
    .presentation = {
#ifdef COMPOSITION_MAIN_CONFIG_PRESENTATION_TASK_WIFIMAN_STA_RECONNECT_ENABLE
        .wifiman_sta_reconnect_task_name        = PRES_TASK_WIFIMAN_STA_RECONNECT_DEFAULT_TASK_NAME,
        .wifiman_sta_reconnect_task_stack_size  = PRES_TASK_WIFIMAN_STA_RECONNECT_DEFAULT_STACK_SIZE,
        .wifiman_sta_reconnect_task_priority    = PRES_TASK_WIFIMAN_STA_RECONNECT_DEFAULT_PRIORITY,
        .wifiman_sta_reconnect_task_interval_ms = PRES_TASK_WIFIMAN_STA_RECONNECT_DEFAULT_INTERVAL_MS,
#endif /* COMPOSITION_MAIN_CONFIG_PRESENTATION_TASK_WIFIMAN_STA_RECONNECT_ENABLE */
    },
};
