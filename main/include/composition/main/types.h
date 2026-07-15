#ifndef COMPOSITION_MAIN_TYPES_H
#define COMPOSITION_MAIN_TYPES_H

#include "config.h"                                         // IWYU pragma: keep
#include "domain/contracts/device/ethernet.h"               // IWYU pragma: keep
#include "domain/contracts/device/wifi.h"                   // IWYU pragma: keep
#include "domain/contracts/logger/leveled.h"                // IWYU pragma: keep
#include "domain/contracts/messaging/publish.h"             // IWYU pragma: keep
#include "domain/contracts/messaging/subscribe.h"           // IWYU pragma: keep
#include "domain/contracts/network/interface.h"             // IWYU pragma: keep
#include "domain/contracts/repository/preloaded.h"          // IWYU pragma: keep
#include "domain/contracts/repository/wifi.h"               // IWYU pragma: keep
#include "domain/contracts/system/info.h"                   // IWYU pragma: keep
#include "domain/contracts/system/restart.h"                // IWYU pragma: keep
#include "domain/contracts/system/update.h"                 // IWYU pragma: keep
#include "domain/usecases/netif.h"                          // IWYU pragma: keep
#include "domain/usecases/ota.h"                            // IWYU pragma: keep
#include "domain/usecases/settings.h"                       // IWYU pragma: keep
#include "domain/usecases/wifiman.h"                        // IWYU pragma: keep
#include "driver/i2c_types.h"                               // IWYU pragma: keep
#include "esp_eth_driver.h"                                 // IWYU pragma: keep
#include "esp_eth_mac.h"                                    // IWYU pragma: keep
#include "esp_eth_phy.h"                                    // IWYU pragma: keep
#include "esp_http_server.h"                                // IWYU pragma: keep
#include "mqtt_client.h"                                    // IWYU pragma: keep
#include "nvs.h"                                            // IWYU pragma: keep
#include "presentation/http/handler/netif_types.h"          // IWYU pragma: keep
#include "presentation/http/handler/settings_types.h"       // IWYU pragma: keep
#include "presentation/http/handler/wifiman_types.h"        // IWYU pragma: keep
#include "presentation/task/wifiman_sta_reconnect/types.h"  // IWYU pragma: keep
#include "sdmmc_cmd.h"                                      // IWYU pragma: keep

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
    char*                    mqtt_client_url;
    char*                    mqtt_client_id;
    char*                    mqtt_client_lwt_topic;
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_MQTT_CLIENT_ENABLE */
} cmp_main_driver_t;

typedef struct {
#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_LOGGER_LEVELED_STDIO_ENABLE
    dom_contracts_logger_leveled_t* logger;
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_LOGGER_LEVELED_STDIO_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_PUBLISH_ENABLE
    dom_contracts_messaging_publish_t* messaging_publish;
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_PUBLISH_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_SUBSCRIBE_ENABLE
    dom_contracts_messaging_subscribe_t* messaging_subscribe;
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_SUBSCRIBE_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_WIFI_ENABLE
    dom_contracts_device_wifi_t* wifi;
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_WIFI_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_ETHERNET_ENABLE
    dom_contracts_device_ethernet_t* ethernet;
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_ETHERNET_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_NETWORK_INTERFACE_ENABLE
    dom_contracts_network_interface_t* network_interface;
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_NETWORK_INTERFACE_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_REPOSITORY_PRELOADED_ENABLE
    dom_contracts_repository_preloaded_t* preloaded_repository;
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_REPOSITORY_PRELOADED_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_REPOSITORY_WIFI_ENABLE
    dom_contracts_repository_wifi_t* wifi_repository;
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_REPOSITORY_WIFI_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_INFO_ENABLE
    dom_contracts_system_info_t* system_info;
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_INFO_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_RESTART_ENABLE
    dom_contracts_system_restart_t* system_restart;
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_RESTART_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_UPDATE_ENABLE
    dom_contracts_system_update_t* system_update;
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_UPDATE_ENABLE */
} cmp_main_infrastructure_t;

typedef struct {
#ifdef COMPOSITION_MAIN_CONFIG_APPLICATION_SETTINGS_ENABLE
    dom_usecases_settings_t* settings;
#endif /* COMPOSITION_MAIN_CONFIG_APPLICATION_SETTINGS_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_APPLICATION_NETIF_ENABLE
    dom_usecases_netif_t* netif;
#endif /* COMPOSITION_MAIN_CONFIG_APPLICATION_NETIF_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_APPLICATION_WIFIMAN_ENABLE
    dom_usecases_wifiman_t* wifiman;
#endif /* COMPOSITION_MAIN_CONFIG_APPLICATION_WIFIMAN_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_APPLICATION_OTA_ENABLE
    dom_usecases_ota_t* ota;
#endif /* COMPOSITION_MAIN_CONFIG_APPLICATION_OTA_ENABLE */
} cmp_main_application_t;

typedef struct {
#ifdef COMPOSITION_MAIN_CONFIG_PRESENTATION_HTTP_NETIF_ENABLE
    pres_http_handler_netif_t netif_http_handler;
#endif /* COMPOSITION_MAIN_CONFIG_PRESENTATION_HTTP_NETIF_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_PRESENTATION_HTTP_SETTINGS_ENABLE
    pres_http_handler_settings_t settings_http_handler;
#endif /* COMPOSITION_MAIN_CONFIG_PRESENTATION_HTTP_SETTINGS_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_PRESENTATION_HTTP_WIFIMAN_ENABLE
    pres_http_handler_wifiman_t wifiman_http_handler;
#endif /* COMPOSITION_MAIN_CONFIG_PRESENTATION_HTTP_WIFIMAN_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_PRESENTATION_TASK_WIFIMAN_STA_RECONNECT_ENABLE
    pres_task_wifiman_sta_reconnect_t* wifiman_sta_reconnect_task;
#endif /* COMPOSITION_MAIN_CONFIG_PRESENTATION_TASK_WIFIMAN_STA_RECONNECT_ENABLE */
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
