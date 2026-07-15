#include "composition/main/application.h"  // IWYU pragma: keep

#include "application/netif/impl.h"     // IWYU pragma: keep
#include "application/ota/impl.h"       // IWYU pragma: keep
#include "application/settings/impl.h"  // IWYU pragma: keep
#include "application/wifiman/impl.h"   // IWYU pragma: keep
#include "composition/main/config.h"    // IWYU pragma: keep
#include "domain/models/error.h"        // IWYU pragma: keep
#include "esp_log.h"                    // IWYU pragma: keep

#define TAG_PATH "main/application"

/* Init Flags for Deinitizalization Sequence */

static bool init_settings = false;
static bool init_netif    = false;
static bool init_wifiman  = false;
static bool init_ota      = false;

dom_models_error_t cmp_main_application_init(cmp_main_launcher_t* launcher) {
    const char* tag = TAG_PATH "/init";

    if (!launcher) {
        ESP_LOGE(tag, "Invalid launcher");
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    /* Settings */

#ifdef COMPOSITION_MAIN_CONFIG_APPLICATION_SETTINGS_ENABLE

#if !defined(COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_LOGGER_LEVELED_STDIO_ENABLE) || \
    !defined(COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_REPOSITORY_PRELOADED_ENABLE) || \
    !defined(COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_INFO_ENABLE) ||          \
    !defined(COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_RESTART_ENABLE)
    ESP_LOGE(tag, "Settings dependencies are disabled");
    return DOMAIN_MODELS_ERROR_BAD_STATE;
#else
    if (!launcher->infrastructure.logger ||
        !launcher->infrastructure.preloaded_repository ||
        !launcher->infrastructure.system_info ||
        !launcher->infrastructure.system_restart) {
        ESP_LOGE(tag, "Settings dependencies are not initialized");
        return DOMAIN_MODELS_ERROR_BAD_STATE;
    }

    app_settings_impl_cfg_t settings_cfg = {
        .logger               = launcher->infrastructure.logger,
        .preloaded_repository = launcher->infrastructure.preloaded_repository,
        .system_info          = launcher->infrastructure.system_info,
        .system_restart       = launcher->infrastructure.system_restart,
    };
    launcher->application.settings = app_settings_impl_new(&settings_cfg);
    if (!launcher->application.settings) {
        ESP_LOGE(tag, "Failed to create Settings");
        cmp_main_application_deinit(launcher);
        return DOMAIN_MODELS_ERROR_MALLOC_FAILED;
    }

    init_settings = true;
    ESP_LOGI(tag, "Settings created");
#endif /* Settings dependencies */

#endif /* COMPOSITION_MAIN_CONFIG_APPLICATION_SETTINGS_ENABLE */

    /* Netif */

#ifdef COMPOSITION_MAIN_CONFIG_APPLICATION_NETIF_ENABLE

#if !defined(COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_LOGGER_LEVELED_STDIO_ENABLE) || \
    !defined(COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_NETWORK_INTERFACE_ENABLE)
    ESP_LOGE(tag, "Netif dependencies are disabled");
    cmp_main_application_deinit(launcher);
    return DOMAIN_MODELS_ERROR_BAD_STATE;
#else
    if (!launcher->infrastructure.logger ||
        !launcher->infrastructure.network_interface) {
        ESP_LOGE(tag, "Netif dependencies are not initialized");
        cmp_main_application_deinit(launcher);
        return DOMAIN_MODELS_ERROR_BAD_STATE;
    }

    app_netif_impl_cfg_t netif_cfg = {
        .logger            = launcher->infrastructure.logger,
        .network_interface = launcher->infrastructure.network_interface,
    };
    launcher->application.netif = app_netif_impl_new(&netif_cfg);
    if (!launcher->application.netif) {
        ESP_LOGE(tag, "Failed to create Netif");
        cmp_main_application_deinit(launcher);
        return DOMAIN_MODELS_ERROR_MALLOC_FAILED;
    }

    init_netif = true;
    ESP_LOGI(tag, "Netif created");
#endif /* Netif dependencies */

#endif /* COMPOSITION_MAIN_CONFIG_APPLICATION_NETIF_ENABLE */

    /* WiFiMan */

#ifdef COMPOSITION_MAIN_CONFIG_APPLICATION_WIFIMAN_ENABLE

#if !defined(COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_LOGGER_LEVELED_STDIO_ENABLE) || \
    !defined(COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_DEVICE_WIFI_ENABLE) ||          \
    !defined(COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_NETWORK_INTERFACE_ENABLE) ||    \
    !defined(COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_REPOSITORY_PRELOADED_ENABLE) || \
    !defined(COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_REPOSITORY_WIFI_ENABLE)
    ESP_LOGE(tag, "WiFiMan dependencies are disabled");
    cmp_main_application_deinit(launcher);
    return DOMAIN_MODELS_ERROR_BAD_STATE;
#else
    if (!launcher->infrastructure.logger ||
        !launcher->infrastructure.wifi ||
        !launcher->infrastructure.network_interface ||
        !launcher->infrastructure.preloaded_repository ||
        !launcher->infrastructure.wifi_repository) {
        ESP_LOGE(tag, "WiFiMan dependencies are not initialized");
        cmp_main_application_deinit(launcher);
        return DOMAIN_MODELS_ERROR_BAD_STATE;
    }

    app_wifiman_impl_cfg_t wifiman_cfg = {
        .logger                 = launcher->infrastructure.logger,
        .wifi                   = launcher->infrastructure.wifi,
        .wifi_repository        = launcher->infrastructure.wifi_repository,
        .preloaded_repository   = launcher->infrastructure.preloaded_repository,
        .network_interface      = launcher->infrastructure.network_interface,
        .reconnect_max_trials   = cmp_main_config.application.wifiman_reconnect_max_trials,
        .ap_auto_manage_enabled = cmp_main_config.application.wifiman_ap_auto_manage_enabled,
    };
    launcher->application.wifiman = app_wifiman_impl_new(&wifiman_cfg);
    if (!launcher->application.wifiman) {
        ESP_LOGE(tag, "Failed to create WiFiMan");
        cmp_main_application_deinit(launcher);
        return DOMAIN_MODELS_ERROR_MALLOC_FAILED;
    }

    dom_models_error_t err = launcher->application.wifiman->start(launcher->application.wifiman);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ESP_LOGE(tag, "Failed to start WiFiMan: %s", dom_models_error_str(err));
        cmp_main_application_deinit(launcher);
        return err;
    }

    init_wifiman = true;
    ESP_LOGI(tag, "WiFiMan started");
#endif /* WiFiMan dependencies */

#endif /* COMPOSITION_MAIN_CONFIG_APPLICATION_WIFIMAN_ENABLE */

    /* OTA */

#ifdef COMPOSITION_MAIN_CONFIG_APPLICATION_OTA_ENABLE

#if !defined(COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_LOGGER_LEVELED_STDIO_ENABLE) || \
    !defined(COMPOSITION_MAIN_CONFIG_INFRASTRUCTURE_SYSTEM_UPDATE_ENABLE)
    ESP_LOGE(tag, "OTA dependencies are disabled");
    cmp_main_application_deinit(launcher);
    return DOMAIN_MODELS_ERROR_BAD_STATE;
#else
    if (!launcher->infrastructure.logger ||
        !launcher->infrastructure.system_update) {
        ESP_LOGE(tag, "OTA dependencies are not initialized");
        cmp_main_application_deinit(launcher);
        return DOMAIN_MODELS_ERROR_BAD_STATE;
    }

    app_ota_impl_cfg_t ota_cfg = {
        .logger = launcher->infrastructure.logger,
        .update = launcher->infrastructure.system_update,
    };
    launcher->application.ota = app_ota_impl_new(&ota_cfg);
    if (!launcher->application.ota) {
        ESP_LOGE(tag, "Failed to create OTA");
        cmp_main_application_deinit(launcher);
        return DOMAIN_MODELS_ERROR_MALLOC_FAILED;
    }

    init_ota = true;
    ESP_LOGI(tag, "OTA created");
#endif /* OTA dependencies */

#endif /* COMPOSITION_MAIN_CONFIG_APPLICATION_OTA_ENABLE */

    return DOMAIN_MODELS_ERROR_OK;
}

void cmp_main_application_deinit(cmp_main_launcher_t* launcher) {
    const char* tag = TAG_PATH "/deinit";

    if (!launcher) {
        return;
    }

#ifdef COMPOSITION_MAIN_CONFIG_APPLICATION_OTA_ENABLE
    if (init_ota) {
        init_ota = false;
    }
    if (launcher->application.ota) {
        app_ota_impl_delete(launcher->application.ota);
        launcher->application.ota = NULL;
    }
#endif /* COMPOSITION_MAIN_CONFIG_APPLICATION_OTA_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_APPLICATION_WIFIMAN_ENABLE
    if (init_wifiman) {
        dom_models_error_t err = launcher->application.wifiman->stop(launcher->application.wifiman);
        if (err != DOMAIN_MODELS_ERROR_OK) {
            ESP_LOGE(tag, "Failed to stop WiFiMan: %s", dom_models_error_str(err));
        }
        init_wifiman = false;
    }
    if (launcher->application.wifiman) {
        app_wifiman_impl_delete(launcher->application.wifiman);
        launcher->application.wifiman = NULL;
    }
#endif /* COMPOSITION_MAIN_CONFIG_APPLICATION_WIFIMAN_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_APPLICATION_NETIF_ENABLE
    if (init_netif) {
        init_netif = false;
    }
    if (launcher->application.netif) {
        app_netif_impl_delete(launcher->application.netif);
        launcher->application.netif = NULL;
    }
#endif /* COMPOSITION_MAIN_CONFIG_APPLICATION_NETIF_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_APPLICATION_SETTINGS_ENABLE
    if (init_settings) {
        init_settings = false;
    }
    if (launcher->application.settings) {
        app_settings_impl_delete(launcher->application.settings);
        launcher->application.settings = NULL;
    }
#endif /* COMPOSITION_MAIN_CONFIG_APPLICATION_SETTINGS_ENABLE */
}
