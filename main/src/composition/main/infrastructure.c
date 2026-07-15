#include "composition/main/infrastructure.h"  // IWYU pragma: keep

#include "composition/main/config.h"                           // IWYU pragma: keep
#include "domain/models/error.h"                               // IWYU pragma: keep
#include "domain/models/preloaded.h"                           // IWYU pragma: keep
#include "esp_log.h"                                           // IWYU pragma: keep
#include "infrastructure/device/ethernet/esp_w5500_impl.h"     // IWYU pragma: keep
#include "infrastructure/device/ethernet/stub_impl.h"          // IWYU pragma: keep
#include "infrastructure/device/wifi/esp_wifi_impl.h"          // IWYU pragma: keep
#include "infrastructure/device/wifi/stub_impl.h"              // IWYU pragma: keep
#include "infrastructure/logger/leveled/stdio_impl.h"          // IWYU pragma: keep
#include "infrastructure/messaging/publish/esp_mqtt_impl.h"    // IWYU pragma: keep
#include "infrastructure/messaging/publish/stub_impl.h"        // IWYU pragma: keep
#include "infrastructure/messaging/subscribe/esp_mqtt_impl.h"  // IWYU pragma: keep
#include "infrastructure/messaging/subscribe/stub_impl.h"      // IWYU pragma: keep
#include "infrastructure/network/interface/esp_netif_impl.h"   // IWYU pragma: keep
#include "infrastructure/network/interface/stub_impl.h"        // IWYU pragma: keep
#include "infrastructure/repository/preloaded/nvs_impl.h"      // IWYU pragma: keep
#include "infrastructure/repository/preloaded/stub_impl.h"     // IWYU pragma: keep
#include "infrastructure/repository/wifi/nvs_impl.h"           // IWYU pragma: keep
#include "infrastructure/repository/wifi/stub_impl.h"          // IWYU pragma: keep
#include "infrastructure/system/info/esp_impl.h"               // IWYU pragma: keep
#include "infrastructure/system/restart/esp_impl.h"            // IWYU pragma: keep
#include "infrastructure/system/update/esp_https_impl.h"       // IWYU pragma: keep
#include "infrastructure/system/update/stub_impl.h"            // IWYU pragma: keep

#define TAG_PATH "main/infrastructure"

/* Init Flags for Deinitizalization Sequence */

static bool init_logger               = false;
static bool init_system_info          = false;
static bool init_system_restart       = false;
static bool init_system_update        = false;
static bool init_messaging_publish    = false;
static bool init_messaging_subscribe  = false;
static bool init_wifi                 = false;
static bool init_ethernet             = false;
static bool init_network_interface    = false;
static bool init_preloaded_repository = false;
static bool init_wifi_repository      = false;

dom_models_error_t cmp_main_infrastructure_init(cmp_main_launcher_t* launcher) {
    const char* tag = TAG_PATH "/init";

    if (!launcher) {
        ESP_LOGE(tag, "Invalid launcher");
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    /* Logger */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_LOGGER_LEVELED_STDIO_ENABLE

    inf_logger_leveled_stdio_impl_cfg_t logger_cfg = {
        .level      = cmp_main_config.infrastructure.logger_leveled_stdio_level,
        .cb_max_cnt = cmp_main_config.infrastructure.logger_leveled_stdio_callback_max_count,
    };
    launcher->infrastructure.logger = inf_logger_leveled_stdio_impl_new(&logger_cfg);
    if (!launcher->infrastructure.logger) {
        ESP_LOGE(tag, "Failed to create leveled stdio logger");
        cmp_main_infrastructure_deinit(launcher);
        return DOMAIN_MODELS_ERROR_MALLOC_FAILED;
    }

    init_logger = true;
    ESP_LOGI(tag, "Leveled stdio logger created");

#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_LOGGER_LEVELED_STDIO_ENABLE */

    /* System Info */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_INFO_ENABLE

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_INFO_USE_ESP
    inf_system_info_esp_impl_cfg_t system_info_cfg = {
        .project_name     = cmp_main_config.infrastructure.system_info_esp_project_name,
        .project_version  = cmp_main_config.infrastructure.system_info_esp_project_version,
        .name             = cmp_main_config.infrastructure.system_info_esp_name,
        .type             = cmp_main_config.infrastructure.system_info_esp_type,
        .firmware_version = cmp_main_config.infrastructure.system_info_esp_firmware_version,
    };
    launcher->infrastructure.system_info = inf_system_info_esp_impl_new(&system_info_cfg);
#else
    ESP_LOGE(tag, "No system info infrastructure backend configured");
    cmp_main_infrastructure_deinit(launcher);
    return DOMAIN_MODELS_ERROR_NOT_SUPPORTED;
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_INFO_USE_ESP */

    if (!launcher->infrastructure.system_info) {
        ESP_LOGE(tag, "Failed to create system info");
        cmp_main_infrastructure_deinit(launcher);
        return DOMAIN_MODELS_ERROR_MALLOC_FAILED;
    }

    init_system_info = true;
    ESP_LOGI(tag, "System info created");

#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_INFO_ENABLE */

    /* System Restart */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_RESTART_ENABLE

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_RESTART_USE_ESP
    inf_system_restart_esp_impl_cfg_t system_restart_cfg = INF_SYSTEM_RESTART_ESP_IMPL_CFG_DEFAULT();
    launcher->infrastructure.system_restart              = inf_system_restart_esp_impl_new(&system_restart_cfg);
#else
    ESP_LOGE(tag, "No system restart infrastructure backend configured");
    cmp_main_infrastructure_deinit(launcher);
    return DOMAIN_MODELS_ERROR_NOT_SUPPORTED;
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_RESTART_USE_ESP */

    if (!launcher->infrastructure.system_restart) {
        ESP_LOGE(tag, "Failed to create system restart");
        cmp_main_infrastructure_deinit(launcher);
        return DOMAIN_MODELS_ERROR_MALLOC_FAILED;
    }

    init_system_restart = true;
    ESP_LOGI(tag, "System restart created");

#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_RESTART_ENABLE */

    /* System Update */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_UPDATE_ENABLE

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_UPDATE_USE_ESP_HTTPS
    inf_system_update_esp_https_impl_cfg_t system_update_cfg = {
        .http_timeout_ms             = cmp_main_config.infrastructure.system_update_esp_https_http_timeout_ms,
        .http_read_buffer_size       = cmp_main_config.infrastructure.system_update_esp_https_http_read_buffer_size,
        .max_empty_read_count        = cmp_main_config.infrastructure.system_update_esp_https_max_empty_read_count,
        .cert_pem                    = cmp_main_config.infrastructure.system_update_esp_https_cert_pem,
        .keep_alive_enable           = cmp_main_config.infrastructure.system_update_esp_https_keep_alive_enable,
        .skip_cert_common_name_check = cmp_main_config.infrastructure.system_update_esp_https_skip_cert_common_name_check,
    };
    launcher->infrastructure.system_update = inf_system_update_esp_https_impl_new(&system_update_cfg);
#else
    inf_system_update_stub_impl_cfg_t system_update_cfg = INF_SYSTEM_UPDATE_STUB_IMPL_CFG_DEFAULT();
    launcher->infrastructure.system_update              = inf_system_update_stub_impl_new(&system_update_cfg);
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_UPDATE_USE_ESP_HTTPS */

    if (!launcher->infrastructure.system_update) {
        ESP_LOGE(tag, "Failed to create system update");
        cmp_main_infrastructure_deinit(launcher);
        return DOMAIN_MODELS_ERROR_MALLOC_FAILED;
    }

    init_system_update = true;
    ESP_LOGI(tag, "System update created");

#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_UPDATE_ENABLE */

    /* Messaging Publish */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_PUBLISH_ENABLE

#if defined(COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_PUBLISH_USE_ESP_MQTT) && \
    !defined(COMPOSITION_MAIN_CONFIG_DRIVER_MQTT_CLIENT_ENABLE)
    ESP_LOGE(tag, "ESP-MQTT messaging publish infrastructure requires MQTT client driver");
    cmp_main_infrastructure_deinit(launcher);
    return DOMAIN_MODELS_ERROR_BAD_STATE;
#else
#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_PUBLISH_USE_ESP_MQTT
    inf_messaging_publish_esp_mqtt_impl_cfg_t messaging_publish_cfg = {
        .mqtt_client           = launcher->driver.mqtt_client_handle,
        .device_id_str         = dom_models_preloaded_data.device_id_str,
        .qos                   = cmp_main_config.infrastructure.messaging_publish_esp_mqtt_qos,
        .registration_retained = cmp_main_config.infrastructure.messaging_publish_esp_mqtt_registration_retained,
        .status_retained       = cmp_main_config.infrastructure.messaging_publish_esp_mqtt_status_retained,
        .log_retained          = cmp_main_config.infrastructure.messaging_publish_esp_mqtt_log_retained,
    };
    launcher->infrastructure.messaging_publish = inf_messaging_publish_esp_mqtt_impl_new(&messaging_publish_cfg);
#else
    inf_messaging_publish_stub_impl_cfg_t messaging_publish_cfg = INF_MESSAGING_PUBLISH_STUB_IMPL_CFG_DEFAULT();
    launcher->infrastructure.messaging_publish                  = inf_messaging_publish_stub_impl_new(&messaging_publish_cfg);
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_PUBLISH_USE_ESP_MQTT */

    if (!launcher->infrastructure.messaging_publish) {
        ESP_LOGE(tag, "Failed to create messaging publish");
        cmp_main_infrastructure_deinit(launcher);
        return DOMAIN_MODELS_ERROR_MALLOC_FAILED;
    }

    init_messaging_publish = true;
    ESP_LOGI(tag, "Messaging publish created");
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_PUBLISH_USE_ESP_MQTT dependency */

#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_PUBLISH_ENABLE */

    /* Messaging Subscribe */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_SUBSCRIBE_ENABLE

#if defined(COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_SUBSCRIBE_USE_ESP_MQTT) && \
    !defined(COMPOSITION_MAIN_CONFIG_DRIVER_MQTT_CLIENT_ENABLE)
    ESP_LOGE(tag, "ESP-MQTT messaging subscribe infrastructure requires MQTT client driver");
    cmp_main_infrastructure_deinit(launcher);
    return DOMAIN_MODELS_ERROR_BAD_STATE;
#else
#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_SUBSCRIBE_USE_ESP_MQTT
    inf_messaging_subscribe_esp_mqtt_impl_cfg_t messaging_subscribe_cfg = {
        .mqtt_client   = launcher->driver.mqtt_client_handle,
        .device_id_str = dom_models_preloaded_data.device_id_str,
        .qos           = cmp_main_config.infrastructure.messaging_subscribe_esp_mqtt_qos,
    };
    launcher->infrastructure.messaging_subscribe = inf_messaging_subscribe_esp_mqtt_impl_new(&messaging_subscribe_cfg);
#else
    inf_messaging_subscribe_stub_impl_cfg_t messaging_subscribe_cfg = INF_MESSAGING_SUBSCRIBE_STUB_IMPL_CFG_DEFAULT();
    launcher->infrastructure.messaging_subscribe                    = inf_messaging_subscribe_stub_impl_new(&messaging_subscribe_cfg);
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_SUBSCRIBE_USE_ESP_MQTT */

    if (!launcher->infrastructure.messaging_subscribe) {
        ESP_LOGE(tag, "Failed to create messaging subscribe");
        cmp_main_infrastructure_deinit(launcher);
        return DOMAIN_MODELS_ERROR_MALLOC_FAILED;
    }

    init_messaging_subscribe = true;
    ESP_LOGI(tag, "Messaging subscribe created");
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_SUBSCRIBE_USE_ESP_MQTT dependency */

#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_SUBSCRIBE_ENABLE */

    /* WiFi Device */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_WIFI_ENABLE

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_WIFI_USE_ESP_WIFI
    inf_device_wifi_esp_wifi_impl_cfg_t wifi_cfg = {
        .sta_if_key             = cmp_main_config.infrastructure.wifi_esp_wifi_sta_if_key,
        .ap_if_key              = cmp_main_config.infrastructure.wifi_esp_wifi_ap_if_key,
        .register_event_handler = cmp_main_config.infrastructure.wifi_esp_wifi_register_event_handler,
    };
    launcher->infrastructure.wifi = inf_device_wifi_esp_wifi_impl_new(&wifi_cfg);
#else
    inf_device_wifi_stub_impl_cfg_t wifi_cfg = INF_DEVICE_WIFI_STUB_IMPL_CFG_DEFAULT();
    launcher->infrastructure.wifi            = inf_device_wifi_stub_impl_new(&wifi_cfg);
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_WIFI_USE_ESP_WIFI */

    if (!launcher->infrastructure.wifi) {
        ESP_LOGE(tag, "Failed to create WiFi device");
        cmp_main_infrastructure_deinit(launcher);
        return DOMAIN_MODELS_ERROR_MALLOC_FAILED;
    }

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_WIFI_USE_ESP_WIFI
    dom_models_error_t err = inf_device_wifi_esp_wifi_impl_init(launcher->infrastructure.wifi);
#else
    dom_models_error_t err = inf_device_wifi_stub_impl_init(launcher->infrastructure.wifi);
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_WIFI_USE_ESP_WIFI */
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ESP_LOGE(tag, "Failed to initialize WiFi device: %s", dom_models_error_str(err));
        cmp_main_infrastructure_deinit(launcher);
        return err;
    }

    init_wifi = true;
    ESP_LOGI(tag, "WiFi device initialized");

#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_WIFI_ENABLE */

    /* Ethernet Device */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_ETHERNET_ENABLE

#if defined(COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_ETHERNET_USE_ESP_W5500) && \
    (!defined(COMPOSITION_MAIN_CONFIG_DRIVER_ETHERNET_ENABLE) || !defined(COMPOSITION_MAIN_CONFIG_DRIVER_ETHERNET_USE_W5500))
    ESP_LOGE(tag, "W5500 Ethernet infrastructure requires W5500 Ethernet driver");
    cmp_main_infrastructure_deinit(launcher);
    return DOMAIN_MODELS_ERROR_BAD_STATE;
#else
#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_ETHERNET_USE_ESP_W5500
    inf_device_ethernet_esp_w5500_impl_cfg_t ethernet_cfg = {
        .eth_handle                = launcher->driver.ethernet_w5500_handle,
        .if_key                    = cmp_main_config.infrastructure.ethernet_esp_w5500_if_key,
        .if_desc                   = cmp_main_config.infrastructure.ethernet_esp_w5500_if_desc,
        .route_prio                = cmp_main_config.infrastructure.ethernet_esp_w5500_route_prio,
        .attach_netif_glue         = cmp_main_config.infrastructure.ethernet_esp_w5500_attach_netif_glue,
        .register_event_handler    = cmp_main_config.infrastructure.ethernet_esp_w5500_register_event_handler,
        .register_ip_event_handler = cmp_main_config.infrastructure.ethernet_esp_w5500_register_ip_event_handler,
    };
    launcher->infrastructure.ethernet = inf_device_ethernet_esp_w5500_impl_new(&ethernet_cfg);
#else
    inf_device_ethernet_stub_impl_cfg_t ethernet_cfg = INF_DEVICE_ETHERNET_STUB_IMPL_CFG_DEFAULT();
    launcher->infrastructure.ethernet                = inf_device_ethernet_stub_impl_new(&ethernet_cfg);
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_ETHERNET_USE_ESP_W5500 */

    if (!launcher->infrastructure.ethernet) {
        ESP_LOGE(tag, "Failed to create Ethernet device");
        cmp_main_infrastructure_deinit(launcher);
        return DOMAIN_MODELS_ERROR_MALLOC_FAILED;
    }

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_ETHERNET_USE_ESP_W5500
    dom_models_error_t ethernet_err = inf_device_ethernet_esp_w5500_impl_init(launcher->infrastructure.ethernet);
#else
    dom_models_error_t ethernet_err = inf_device_ethernet_stub_impl_init(launcher->infrastructure.ethernet);
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_ETHERNET_USE_ESP_W5500 */
    if (ethernet_err != DOMAIN_MODELS_ERROR_OK) {
        ESP_LOGE(tag, "Failed to initialize Ethernet device: %s", dom_models_error_str(ethernet_err));
        cmp_main_infrastructure_deinit(launcher);
        return ethernet_err;
    }

    init_ethernet = true;
    ESP_LOGI(tag, "Ethernet device initialized");
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_ETHERNET_USE_ESP_W5500 dependency */

#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_ETHERNET_ENABLE */

    /* Network Interface */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_NETWORK_INTERFACE_ENABLE

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_NETWORK_INTERFACE_USE_ESP_NETIF
    inf_network_interface_esp_netif_impl_cfg_t network_interface_cfg = {
        .sta_if_key = cmp_main_config.infrastructure.network_interface_esp_netif_sta_if_key,
        .eth_if_key = cmp_main_config.infrastructure.network_interface_esp_netif_eth_if_key,
    };
    launcher->infrastructure.network_interface = inf_network_interface_esp_netif_impl_new(&network_interface_cfg);
#else
    inf_network_interface_stub_impl_cfg_t network_interface_cfg = INF_NETWORK_INTERFACE_STUB_IMPL_CFG_DEFAULT();
    launcher->infrastructure.network_interface                  = inf_network_interface_stub_impl_new(&network_interface_cfg);
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_NETWORK_INTERFACE_USE_ESP_NETIF */

    if (!launcher->infrastructure.network_interface) {
        ESP_LOGE(tag, "Failed to create network interface repository");
        cmp_main_infrastructure_deinit(launcher);
        return DOMAIN_MODELS_ERROR_MALLOC_FAILED;
    }

    init_network_interface = true;
    ESP_LOGI(tag, "Network interface repository created");

#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_NETWORK_INTERFACE_ENABLE */

    /* Preloaded Repository */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_REPOSITORY_PRELOADED_ENABLE

#if defined(COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_REPOSITORY_PRELOADED_USE_NVS) && defined(COMPOSITION_MAIN_CONFIG_DRIVER_NVS_ENABLE)
    inf_repository_preloaded_nvs_impl_cfg_t preloaded_repository_cfg = {
        .nvs = launcher->driver.nvs_handle,
    };
    launcher->infrastructure.preloaded_repository = inf_repository_preloaded_nvs_impl_new(&preloaded_repository_cfg);
#else
    inf_repository_preloaded_stub_impl_cfg_t preloaded_repository_cfg = {
        .device_id     = dom_models_preloaded_data.device_id,
        .device_id_str = dom_models_preloaded_data.device_id_str,
        .wifi_ap_ssid  = dom_models_preloaded_data.wifi_ap_ssid,
        .wifi_ap_pass  = dom_models_preloaded_data.wifi_ap_pass,
        .mqtt_proto    = dom_models_preloaded_data.mqtt_proto,
        .mqtt_host     = dom_models_preloaded_data.mqtt_host,
        .mqtt_port     = dom_models_preloaded_data.mqtt_port,
        .mqtt_user     = dom_models_preloaded_data.mqtt_user,
        .mqtt_pass     = dom_models_preloaded_data.mqtt_pass,
    };
    launcher->infrastructure.preloaded_repository = inf_repository_preloaded_stub_impl_new(&preloaded_repository_cfg);
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_REPOSITORY_PRELOADED_USE_NVS && COMPOSITION_MAIN_CONFIG_DRIVER_NVS_ENABLE */

    if (!launcher->infrastructure.preloaded_repository) {
        ESP_LOGE(tag, "Failed to create preloaded repository");
        cmp_main_infrastructure_deinit(launcher);
        return DOMAIN_MODELS_ERROR_MALLOC_FAILED;
    }

    init_preloaded_repository = true;
    ESP_LOGI(tag, "Preloaded repository created");

#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_REPOSITORY_PRELOADED_ENABLE */

    /* WiFi Repository */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_REPOSITORY_WIFI_ENABLE

#if defined(COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_REPOSITORY_WIFI_USE_NVS) && defined(COMPOSITION_MAIN_CONFIG_DRIVER_NVS_ENABLE)
    inf_repository_wifi_nvs_impl_cfg_t wifi_repository_cfg = {
        .nvs = launcher->driver.nvs_handle,
    };
    launcher->infrastructure.wifi_repository = inf_repository_wifi_nvs_impl_new(&wifi_repository_cfg);
#else
    inf_repository_wifi_stub_impl_cfg_t wifi_repository_cfg = INF_REPOSITORY_WIFI_STUB_IMPL_CFG_DEFAULT();
    launcher->infrastructure.wifi_repository                = inf_repository_wifi_stub_impl_new(&wifi_repository_cfg);
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_REPOSITORY_WIFI_USE_NVS && COMPOSITION_MAIN_CONFIG_DRIVER_NVS_ENABLE */

    if (!launcher->infrastructure.wifi_repository) {
        ESP_LOGE(tag, "Failed to create WiFi repository");
        cmp_main_infrastructure_deinit(launcher);
        return DOMAIN_MODELS_ERROR_MALLOC_FAILED;
    }

    init_wifi_repository = true;
    ESP_LOGI(tag, "WiFi repository created");

#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_REPOSITORY_WIFI_ENABLE */

    return DOMAIN_MODELS_ERROR_OK;
}

void cmp_main_infrastructure_deinit(cmp_main_launcher_t* launcher) {
    if (!launcher) {
        return;
    }

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_REPOSITORY_WIFI_ENABLE
    if (init_wifi_repository) {
#if defined(COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_REPOSITORY_WIFI_USE_NVS) && defined(COMPOSITION_MAIN_CONFIG_DRIVER_NVS_ENABLE)
        inf_repository_wifi_nvs_impl_delete(launcher->infrastructure.wifi_repository);
#else
        inf_repository_wifi_stub_impl_delete(launcher->infrastructure.wifi_repository);
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_REPOSITORY_WIFI_USE_NVS && COMPOSITION_MAIN_CONFIG_DRIVER_NVS_ENABLE */
        launcher->infrastructure.wifi_repository = NULL;
        init_wifi_repository                     = false;
    }
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_REPOSITORY_WIFI_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_REPOSITORY_PRELOADED_ENABLE
    if (init_preloaded_repository) {
#if defined(COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_REPOSITORY_PRELOADED_USE_NVS) && defined(COMPOSITION_MAIN_CONFIG_DRIVER_NVS_ENABLE)
        inf_repository_preloaded_nvs_impl_delete(launcher->infrastructure.preloaded_repository);
#else
        inf_repository_preloaded_stub_impl_delete(launcher->infrastructure.preloaded_repository);
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_REPOSITORY_PRELOADED_USE_NVS && COMPOSITION_MAIN_CONFIG_DRIVER_NVS_ENABLE */
        launcher->infrastructure.preloaded_repository = NULL;
        init_preloaded_repository                     = false;
    }
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_REPOSITORY_PRELOADED_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_NETWORK_INTERFACE_ENABLE
    if (init_network_interface) {
#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_NETWORK_INTERFACE_USE_ESP_NETIF
        inf_network_interface_esp_netif_impl_delete(launcher->infrastructure.network_interface);
#else
        inf_network_interface_stub_impl_delete(launcher->infrastructure.network_interface);
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_NETWORK_INTERFACE_USE_ESP_NETIF */
        launcher->infrastructure.network_interface = NULL;
        init_network_interface                     = false;
    }
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_NETWORK_INTERFACE_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_ETHERNET_ENABLE
    if (init_ethernet) {
#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_ETHERNET_USE_ESP_W5500
        inf_device_ethernet_esp_w5500_impl_deinit(launcher->infrastructure.ethernet);
#else
        inf_device_ethernet_stub_impl_deinit(launcher->infrastructure.ethernet);
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_ETHERNET_USE_ESP_W5500 */
        init_ethernet = false;
    }
    if (launcher->infrastructure.ethernet) {
#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_ETHERNET_USE_ESP_W5500
        inf_device_ethernet_esp_w5500_impl_delete(launcher->infrastructure.ethernet);
#else
        inf_device_ethernet_stub_impl_delete(launcher->infrastructure.ethernet);
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_ETHERNET_USE_ESP_W5500 */
        launcher->infrastructure.ethernet = NULL;
    }
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_ETHERNET_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_WIFI_ENABLE
    if (init_wifi) {
#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_WIFI_USE_ESP_WIFI
        inf_device_wifi_esp_wifi_impl_deinit(launcher->infrastructure.wifi);
#else
        inf_device_wifi_stub_impl_deinit(launcher->infrastructure.wifi);
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_WIFI_USE_ESP_WIFI */
        init_wifi = false;
    }
    if (launcher->infrastructure.wifi) {
#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_WIFI_USE_ESP_WIFI
        inf_device_wifi_esp_wifi_impl_delete(launcher->infrastructure.wifi);
#else
        inf_device_wifi_stub_impl_delete(launcher->infrastructure.wifi);
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_WIFI_USE_ESP_WIFI */
        launcher->infrastructure.wifi = NULL;
    }
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_WIFI_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_SUBSCRIBE_ENABLE
    if (init_messaging_subscribe) {
#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_SUBSCRIBE_USE_ESP_MQTT
        inf_messaging_subscribe_esp_mqtt_impl_delete(launcher->infrastructure.messaging_subscribe);
#else
        inf_messaging_subscribe_stub_impl_delete(launcher->infrastructure.messaging_subscribe);
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_SUBSCRIBE_USE_ESP_MQTT */
        launcher->infrastructure.messaging_subscribe = NULL;
        init_messaging_subscribe                     = false;
    }
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_SUBSCRIBE_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_PUBLISH_ENABLE
    if (init_messaging_publish) {
#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_PUBLISH_USE_ESP_MQTT
        inf_messaging_publish_esp_mqtt_impl_delete(launcher->infrastructure.messaging_publish);
#else
        inf_messaging_publish_stub_impl_delete(launcher->infrastructure.messaging_publish);
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_PUBLISH_USE_ESP_MQTT */
        launcher->infrastructure.messaging_publish = NULL;
        init_messaging_publish                     = false;
    }
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_MESSAGING_PUBLISH_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_UPDATE_ENABLE
    if (init_system_update) {
#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_UPDATE_USE_ESP_HTTPS
        inf_system_update_esp_https_impl_delete(launcher->infrastructure.system_update);
#else
        inf_system_update_stub_impl_delete(launcher->infrastructure.system_update);
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_UPDATE_USE_ESP_HTTPS */
        launcher->infrastructure.system_update = NULL;
        init_system_update                     = false;
    }
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_UPDATE_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_RESTART_ENABLE
    if (init_system_restart) {
#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_RESTART_USE_ESP
        inf_system_restart_esp_impl_delete(launcher->infrastructure.system_restart);
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_RESTART_USE_ESP */
        launcher->infrastructure.system_restart = NULL;
        init_system_restart                     = false;
    }
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_RESTART_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_INFO_ENABLE
    if (init_system_info) {
#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_INFO_USE_ESP
        inf_system_info_esp_impl_delete(launcher->infrastructure.system_info);
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_INFO_USE_ESP */
        launcher->infrastructure.system_info = NULL;
        init_system_info                     = false;
    }
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_INFO_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_LOGGER_LEVELED_STDIO_ENABLE
    if (init_logger) {
        inf_logger_leveled_stdio_impl_delete(launcher->infrastructure.logger);
        launcher->infrastructure.logger = NULL;
        init_logger                     = false;
    }
#endif /* COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_LOGGER_LEVELED_STDIO_ENABLE */
}
