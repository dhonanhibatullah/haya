#ifndef COMPOSITION_MAIN_CONFIG_H
#define COMPOSITION_MAIN_CONFIG_H

#include <stdbool.h>  // IWYU pragma: keep
#include <stddef.h>   // IWYU pragma: keep
#include <stdint.h>   // IWYU pragma: keep

#include "domain/models/logger.h"  // IWYU pragma: keep
#include "hal/gpio_types.h"        // IWYU pragma: keep
#include "hal/spi_types.h"         // IWYU pragma: keep
#include "soc/gpio_num.h"          // IWYU pragma: keep

/* Misc. Config Defines */

#define COMPOSITION_MAIN_CONFIG_PRELOADED_WIFI_AP_SSID_USE_DEVICE_ID

/* Driver Config Defines */

#define COMPOSITION_MAIN_CONFIG_DRIVER_ISR_ENABLE
#define COMPOSITION_MAIN_CONFIG_DRIVER_GPIO_ENABLE
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
#define COMPOSITION_MAIN_CONFIG_DRIVER_SNTP_ENABLE

/* Infrastructure Config Defines */

#define COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_LOGGER_LEVELED_STDIO_ENABLE
#define COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_PUBLISH_ENABLE
#define COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_PUBLISH_USE_ESP_MQTT
#define COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_SUBSCRIBE_ENABLE
#define COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_SUBSCRIBE_USE_ESP_MQTT
#define COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_WIFI_ENABLE
#define COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_WIFI_USE_ESP_WIFI
#define COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_ETHERNET_ENABLE
#define COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_ETHERNET_USE_ESP_W5500
#define COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_NETWORK_INTERFACE_ENABLE
#define COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_NETWORK_INTERFACE_USE_ESP_NETIF
#define COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_REPOSITORY_PRELOADED_ENABLE
#define COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_REPOSITORY_PRELOADED_USE_NVS
#define COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_REPOSITORY_WIFI_ENABLE
#define COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_REPOSITORY_WIFI_USE_NVS
#define COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_INFO_ENABLE
#define COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_INFO_USE_ESP
#define COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_RESTART_ENABLE
#define COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_RESTART_USE_ESP
#define COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_UPDATE_ENABLE
#define COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_UPDATE_USE_ESP_HTTPS

/* Application Config Defines */

#define COMPOSITION_MAIN_CONFIG_APPLICATION_SETTINGS_ENABLE
#define COMPOSITION_MAIN_CONFIG_APPLICATION_NETIF_ENABLE
#define COMPOSITION_MAIN_CONFIG_APPLICATION_WIFIMAN_ENABLE
#define COMPOSITION_MAIN_CONFIG_APPLICATION_OTA_ENABLE

/* Presentation Config Defines */

#define COMPOSITION_MAIN_CONFIG_PRESENTATION_HTTP_NETIF_ENABLE
#define COMPOSITION_MAIN_CONFIG_PRESENTATION_HTTP_SETTINGS_ENABLE
#define COMPOSITION_MAIN_CONFIG_PRESENTATION_HTTP_WIFIMAN_ENABLE
#define COMPOSITION_MAIN_CONFIG_PRESENTATION_TASK_WIFIMAN_STA_RECONNECT_ENABLE
#define COMPOSITION_MAIN_CONFIG_PRESENTATION_MQTT_ENABLE

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    gpio_num_t      gpio_num;
    gpio_mode_t     mode;
    gpio_pullup_t   pull_up_en;
    gpio_pulldown_t pull_down_en;
    gpio_int_type_t intr_type;
    bool            initial_output_level;
} cmp_main_config_driver_gpio_t;

typedef struct {
    struct driver {
        /* ISR */
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_ISR_ENABLE
        const int isr_intr_alloc_flag;
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_ISR_ENABLE */

        /* GPIO */
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_GPIO_ENABLE
        const cmp_main_config_driver_gpio_t* gpio_configs;
        const size_t                         gpio_configs_cnt;
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_GPIO_ENABLE */

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

#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_MQTT_CLIENT_ENABLE
        const int   mqtt_client_reconnect_timeout_ms;
        const char* mqtt_client_lwt_msg;
        const int   mqtt_client_lwt_qos;
        const bool  mqtt_client_lwt_retain;
        const int   mqtt_client_buffer_size;
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_MQTT_CLIENT_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_SNTP_ENABLE
        const char* sntp_server;
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_SNTP_ENABLE */
    } driver;

    struct infrastructure {
#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_LOGGER_LEVELED_STDIO_ENABLE
        const dom_models_logger_level_t logger_leveled_stdio_level;
        const unsigned int              logger_leveled_stdio_callback_max_count;
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_LOGGER_LEVELED_STDIO_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_PUBLISH_ENABLE
#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_PUBLISH_USE_ESP_MQTT
        const int  messaging_publish_esp_mqtt_qos;
        const bool messaging_publish_esp_mqtt_registration_retained;
        const bool messaging_publish_esp_mqtt_status_retained;
        const bool messaging_publish_esp_mqtt_log_retained;
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_PUBLISH_USE_ESP_MQTT */
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_PUBLISH_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_SUBSCRIBE_ENABLE
#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_SUBSCRIBE_USE_ESP_MQTT
        const int messaging_subscribe_esp_mqtt_qos;
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_SUBSCRIBE_USE_ESP_MQTT */
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_SUBSCRIBE_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_WIFI_ENABLE
#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_WIFI_USE_ESP_WIFI
        const char* wifi_esp_wifi_sta_if_key;
        const char* wifi_esp_wifi_ap_if_key;
        const bool  wifi_esp_wifi_register_event_handler;
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_WIFI_USE_ESP_WIFI */
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_WIFI_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_ETHERNET_ENABLE
#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_ETHERNET_USE_ESP_W5500
        const char* ethernet_esp_w5500_if_key;
        const char* ethernet_esp_w5500_if_desc;
        const int   ethernet_esp_w5500_route_prio;
        const bool  ethernet_esp_w5500_attach_netif_glue;
        const bool  ethernet_esp_w5500_register_event_handler;
        const bool  ethernet_esp_w5500_register_ip_event_handler;
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_ETHERNET_USE_ESP_W5500 */
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_ETHERNET_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_NETWORK_INTERFACE_ENABLE
#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_NETWORK_INTERFACE_USE_ESP_NETIF
        const char* network_interface_esp_netif_sta_if_key;
        const char* network_interface_esp_netif_eth_if_key;
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_NETWORK_INTERFACE_USE_ESP_NETIF */
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_NETWORK_INTERFACE_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_INFO_ENABLE
#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_INFO_USE_ESP
        const char* system_info_esp_project_name;
        const char* system_info_esp_project_version;
        const char* system_info_esp_name;
        const char* system_info_esp_type;
        const char* system_info_esp_firmware_version;
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_INFO_USE_ESP */
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_INFO_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_UPDATE_ENABLE
#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_UPDATE_USE_ESP_HTTPS
        const int   system_update_esp_https_http_timeout_ms;
        const int   system_update_esp_https_http_read_buffer_size;
        const int   system_update_esp_https_max_empty_read_count;
        const char* system_update_esp_https_cert_pem;
        const bool  system_update_esp_https_keep_alive_enable;
        const bool  system_update_esp_https_skip_cert_common_name_check;
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_UPDATE_USE_ESP_HTTPS */
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_UPDATE_ENABLE */
    } infrastructure;

    struct application {
#ifdef COMPOSITION_MAIN_CONFIG_APPLICATION_WIFIMAN_ENABLE
        const size_t wifiman_reconnect_max_trials;
        const bool   wifiman_ap_auto_manage_enabled;
#endif /* COMPOSITION_MAIN_CONFIG_APPLICATION_WIFIMAN_ENABLE */
    } application;

    struct presentation {
#ifdef COMPOSITION_MAIN_CONFIG_PRESENTATION_TASK_WIFIMAN_STA_RECONNECT_ENABLE
        const char*    wifiman_sta_reconnect_task_name;
        const uint32_t wifiman_sta_reconnect_task_stack_size;
        const uint32_t wifiman_sta_reconnect_task_priority;
        const uint32_t wifiman_sta_reconnect_task_interval_ms;
#endif /* COMPOSITION_MAIN_CONFIG_PRESENTATION_TASK_WIFIMAN_STA_RECONNECT_ENABLE */
    } presentation;

} cmp_main_config_t;

extern const cmp_main_config_t cmp_main_config;

#ifdef __cplusplus
}
#endif

#endif /* COMPOSITION_MAIN_CONFIG_H */
