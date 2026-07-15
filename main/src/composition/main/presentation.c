#include "composition/main/presentation.h"

#include "composition/main/config.h"                       // IWYU pragma: keep
#include "domain/models/error.h"                           // IWYU pragma: keep
#include "esp_err.h"                                       // IWYU pragma: keep
#include "esp_log.h"                                       // IWYU pragma: keep
#include "mqtt_client.h"                                   // IWYU pragma: keep
#include "presentation/http/route/netif.h"                 // IWYU pragma: keep
#include "presentation/http/route/settings.h"              // IWYU pragma: keep
#include "presentation/http/route/wifiman.h"               // IWYU pragma: keep
#include "presentation/mqtt/context.h"                     // IWYU pragma: keep
#include "presentation/mqtt/event/event_handler.h"         // IWYU pragma: keep
#include "presentation/task/wifiman_sta_reconnect/task.h"  // IWYU pragma: keep

#define TAG_PATH "main/presentation"

/* Init Flags for Deinitizalization Sequence */

static bool init_netif_http_routes          = false;
static bool init_settings_http_routes       = false;
static bool init_wifiman_http_routes        = false;
static bool init_wifiman_sta_reconnect_task = false;
static bool init_mqtt_presentation          = false;

dom_models_error_t cmp_main_presentation_init(cmp_main_launcher_t* launcher) {
    const char* tag = TAG_PATH "/init";

    if (!launcher) {
        ESP_LOGE(tag, "Invalid launcher");
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    /* Netif HTTP Routes */

#ifdef COMPOSITION_MAIN_CONFIG_PRESENTATION_HTTP_NETIF_ENABLE

#if !defined(COMPOSITION_MAIN_CONFIG_DRIVER_HTTP_SERVER_ENABLE) || !defined(COMPOSITION_MAIN_CONFIG_APPLICATION_NETIF_ENABLE)
    ESP_LOGE(tag, "Netif HTTP dependencies are disabled");
    cmp_main_presentation_deinit(launcher);
    return DOMAIN_MODELS_ERROR_BAD_STATE;
#else
    if (!launcher->driver.http_server_handle || !launcher->application.netif) {
        ESP_LOGE(tag, "Netif HTTP dependencies are not initialized");
        cmp_main_presentation_deinit(launcher);
        return DOMAIN_MODELS_ERROR_BAD_STATE;
    }

    launcher->presentation.netif_http_handler.netif = launcher->application.netif;

    esp_err_t netif_http_err = pres_http_route_netif_register(
        launcher->driver.http_server_handle,
        &launcher->presentation.netif_http_handler
    );
    if (netif_http_err != ESP_OK) {
        ESP_LOGE(tag, "Failed to register Netif HTTP routes: %s", esp_err_to_name(netif_http_err));
        pres_http_route_netif_unregister(launcher->driver.http_server_handle);
        launcher->presentation.netif_http_handler.netif = NULL;
        cmp_main_presentation_deinit(launcher);
        return DOMAIN_MODELS_ERROR_FAILURE;
    }

    init_netif_http_routes = true;
    ESP_LOGI(tag, "Netif HTTP routes registered");
#endif /* Netif HTTP dependencies */

#endif /* COMPOSITION_MAIN_CONFIG_PRESENTATION_HTTP_NETIF_ENABLE */

    /* Settings HTTP Routes */

#ifdef COMPOSITION_MAIN_CONFIG_PRESENTATION_HTTP_SETTINGS_ENABLE

#if !defined(COMPOSITION_MAIN_CONFIG_DRIVER_HTTP_SERVER_ENABLE) || !defined(COMPOSITION_MAIN_CONFIG_APPLICATION_SETTINGS_ENABLE)
    ESP_LOGE(tag, "Settings HTTP dependencies are disabled");
    cmp_main_presentation_deinit(launcher);
    return DOMAIN_MODELS_ERROR_BAD_STATE;
#else
    if (!launcher->driver.http_server_handle || !launcher->application.settings) {
        ESP_LOGE(tag, "Settings HTTP dependencies are not initialized");
        cmp_main_presentation_deinit(launcher);
        return DOMAIN_MODELS_ERROR_BAD_STATE;
    }

    launcher->presentation.settings_http_handler.settings = launcher->application.settings;

    esp_err_t settings_http_err = pres_http_route_settings_register(
        launcher->driver.http_server_handle,
        &launcher->presentation.settings_http_handler
    );
    if (settings_http_err != ESP_OK) {
        ESP_LOGE(tag, "Failed to register Settings HTTP routes: %s", esp_err_to_name(settings_http_err));
        pres_http_route_settings_unregister(launcher->driver.http_server_handle);
        launcher->presentation.settings_http_handler.settings = NULL;
        cmp_main_presentation_deinit(launcher);
        return DOMAIN_MODELS_ERROR_FAILURE;
    }

    init_settings_http_routes = true;
    ESP_LOGI(tag, "Settings HTTP routes registered");
#endif /* Settings HTTP dependencies */

#endif /* COMPOSITION_MAIN_CONFIG_PRESENTATION_HTTP_SETTINGS_ENABLE */

    /* WiFiMan HTTP Routes */

#ifdef COMPOSITION_MAIN_CONFIG_PRESENTATION_HTTP_WIFIMAN_ENABLE

#if !defined(COMPOSITION_MAIN_CONFIG_DRIVER_HTTP_SERVER_ENABLE) || !defined(COMPOSITION_MAIN_CONFIG_APPLICATION_WIFIMAN_ENABLE)
    ESP_LOGE(tag, "WiFiMan HTTP dependencies are disabled");
    cmp_main_presentation_deinit(launcher);
    return DOMAIN_MODELS_ERROR_BAD_STATE;
#else
    if (!launcher->driver.http_server_handle || !launcher->application.wifiman) {
        ESP_LOGE(tag, "WiFiMan HTTP dependencies are not initialized");
        cmp_main_presentation_deinit(launcher);
        return DOMAIN_MODELS_ERROR_BAD_STATE;
    }

    launcher->presentation.wifiman_http_handler.wifiman = launcher->application.wifiman;

    esp_err_t http_err = pres_http_route_wifiman_register(
        launcher->driver.http_server_handle,
        &launcher->presentation.wifiman_http_handler
    );
    if (http_err != ESP_OK) {
        ESP_LOGE(tag, "Failed to register WiFiMan HTTP routes: %s", esp_err_to_name(http_err));
        pres_http_route_wifiman_unregister(launcher->driver.http_server_handle);
        launcher->presentation.wifiman_http_handler.wifiman = NULL;
        cmp_main_presentation_deinit(launcher);
        return DOMAIN_MODELS_ERROR_FAILURE;
    }

    init_wifiman_http_routes = true;
    ESP_LOGI(tag, "WiFiMan HTTP routes registered");
#endif /* WiFiMan HTTP dependencies */

#endif /* COMPOSITION_MAIN_CONFIG_PRESENTATION_HTTP_WIFIMAN_ENABLE */

    /* WiFiMan STA Reconnect Task */

#ifdef COMPOSITION_MAIN_CONFIG_PRESENTATION_TASK_WIFIMAN_STA_RECONNECT_ENABLE

#ifndef COMPOSITION_MAIN_CONFIG_APPLICATION_WIFIMAN_ENABLE
    ESP_LOGE(tag, "WiFiMan STA reconnect task dependency is disabled");
    cmp_main_presentation_deinit(launcher);
    return DOMAIN_MODELS_ERROR_BAD_STATE;
#else
    if (!launcher->application.wifiman) {
        ESP_LOGE(tag, "WiFiMan STA reconnect task dependency is not initialized");
        cmp_main_presentation_deinit(launcher);
        return DOMAIN_MODELS_ERROR_BAD_STATE;
    }

    pres_task_wifiman_sta_reconnect_cfg_t wifiman_sta_reconnect_task_cfg = {
        .wifiman     = launcher->application.wifiman,
        .task_name   = cmp_main_config.presentation.wifiman_sta_reconnect_task_name,
        .stack_size  = cmp_main_config.presentation.wifiman_sta_reconnect_task_stack_size,
        .priority    = (UBaseType_t)cmp_main_config.presentation.wifiman_sta_reconnect_task_priority,
        .interval_ms = cmp_main_config.presentation.wifiman_sta_reconnect_task_interval_ms,
    };
    launcher->presentation.wifiman_sta_reconnect_task = pres_task_wifiman_sta_reconnect_new(
        &wifiman_sta_reconnect_task_cfg
    );
    if (!launcher->presentation.wifiman_sta_reconnect_task) {
        ESP_LOGE(tag, "Failed to create WiFiMan STA reconnect task");
        cmp_main_presentation_deinit(launcher);
        return DOMAIN_MODELS_ERROR_MALLOC_FAILED;
    }

    dom_models_error_t task_err = pres_task_wifiman_sta_reconnect_start(
        launcher->presentation.wifiman_sta_reconnect_task
    );
    if (task_err != DOMAIN_MODELS_ERROR_OK) {
        ESP_LOGE(tag, "Failed to start WiFiMan STA reconnect task: %s", dom_models_error_str(task_err));
        cmp_main_presentation_deinit(launcher);
        return task_err;
    }

    init_wifiman_sta_reconnect_task = true;
    ESP_LOGI(tag, "WiFiMan STA reconnect task started");
#endif /* COMPOSITION_MAIN_CONFIG_APPLICATION_WIFIMAN_ENABLE */

#endif /* COMPOSITION_MAIN_CONFIG_PRESENTATION_TASK_WIFIMAN_STA_RECONNECT_ENABLE */

    /* MQTT Presentation */

#ifdef COMPOSITION_MAIN_CONFIG_PRESENTATION_MQTT_ENABLE

#if !defined(COMPOSITION_MAIN_CONFIG_DRIVER_MQTT_CLIENT_ENABLE) ||   \
    !defined(COMPOSITION_MAIN_CONFIG_APPLICATION_SETTINGS_ENABLE) || \
    !defined(COMPOSITION_MAIN_CONFIG_APPLICATION_OTA_ENABLE)
    ESP_LOGE(tag, "MQTT presentation dependencies are disabled");
    cmp_main_presentation_deinit(launcher);
    return DOMAIN_MODELS_ERROR_BAD_STATE;
#else
    if (!launcher->driver.mqtt_client_handle ||
        !launcher->infrastructure.logger ||
        !launcher->infrastructure.preloaded_repository ||
        !launcher->application.settings ||
        !launcher->application.ota) {
        ESP_LOGE(tag, "MQTT presentation dependencies are not initialized");
        cmp_main_presentation_deinit(launcher);
        return DOMAIN_MODELS_ERROR_BAD_STATE;
    }

    launcher->presentation.mqtt_context = pres_mqtt_context_new(
        launcher->infrastructure.logger,
        launcher->infrastructure.preloaded_repository,
        launcher->application.settings,
        launcher->application.ota
    );
    if (!launcher->presentation.mqtt_context) {
        ESP_LOGE(tag, "Failed to create MQTT context");
        cmp_main_presentation_deinit(launcher);
        return DOMAIN_MODELS_ERROR_MALLOC_FAILED;
    }

    esp_err_t mqtt_handler_err = esp_mqtt_client_register_event(
        launcher->driver.mqtt_client_handle,
        ESP_EVENT_ANY_ID,
        pres_mqtt_event_handler,
        launcher->presentation.mqtt_context
    );
    if (mqtt_handler_err != ESP_OK) {
        ESP_LOGE(tag, "Failed to register MQTT event handler: %s", esp_err_to_name(mqtt_handler_err));
        cmp_main_presentation_deinit(launcher);
        return DOMAIN_MODELS_ERROR_FAILURE;
    }

    init_mqtt_presentation = true;
    ESP_LOGI(tag, "MQTT presentation event handler registered");
#endif /* MQTT presentation dependencies */

#endif /* COMPOSITION_MAIN_CONFIG_PRESENTATION_MQTT_ENABLE */

    return DOMAIN_MODELS_ERROR_OK;
}

void cmp_main_presentation_deinit(cmp_main_launcher_t* launcher) {
    const char* tag = TAG_PATH "/deinit";

    if (!launcher) {
        return;
    }

#ifdef COMPOSITION_MAIN_CONFIG_PRESENTATION_MQTT_ENABLE
    if (init_mqtt_presentation) {
        if (launcher->driver.mqtt_client_handle) {
            esp_mqtt_client_unregister_event(
                launcher->driver.mqtt_client_handle,
                ESP_EVENT_ANY_ID,
                pres_mqtt_event_handler
            );
        }
        init_mqtt_presentation = false;
    }
    if (launcher->presentation.mqtt_context) {
        pres_mqtt_context_delete(launcher->presentation.mqtt_context);
        launcher->presentation.mqtt_context = NULL;
    }
#endif /* COMPOSITION_MAIN_CONFIG_PRESENTATION_MQTT_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_PRESENTATION_TASK_WIFIMAN_STA_RECONNECT_ENABLE
    if (init_wifiman_sta_reconnect_task) {
        dom_models_error_t err = pres_task_wifiman_sta_reconnect_stop(
            launcher->presentation.wifiman_sta_reconnect_task
        );
        if (err != DOMAIN_MODELS_ERROR_OK) {
            ESP_LOGE(tag, "Failed to stop WiFiMan STA reconnect task: %s", dom_models_error_str(err));
        }
        init_wifiman_sta_reconnect_task = false;
    }
    if (launcher->presentation.wifiman_sta_reconnect_task) {
        pres_task_wifiman_sta_reconnect_delete(launcher->presentation.wifiman_sta_reconnect_task);
        launcher->presentation.wifiman_sta_reconnect_task = NULL;
    }
#endif /* COMPOSITION_MAIN_CONFIG_PRESENTATION_TASK_WIFIMAN_STA_RECONNECT_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_PRESENTATION_HTTP_WIFIMAN_ENABLE
    if (init_wifiman_http_routes) {
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_HTTP_SERVER_ENABLE
        esp_err_t err = pres_http_route_wifiman_unregister(launcher->driver.http_server_handle);
        if (err != ESP_OK) {
            ESP_LOGE(tag, "Failed to unregister WiFiMan HTTP routes: %s", esp_err_to_name(err));
        }
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_HTTP_SERVER_ENABLE */
        launcher->presentation.wifiman_http_handler.wifiman = NULL;
        init_wifiman_http_routes                            = false;
    }
#endif /* COMPOSITION_MAIN_CONFIG_PRESENTATION_HTTP_WIFIMAN_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_PRESENTATION_HTTP_SETTINGS_ENABLE
    if (init_settings_http_routes) {
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_HTTP_SERVER_ENABLE
        esp_err_t err = pres_http_route_settings_unregister(launcher->driver.http_server_handle);
        if (err != ESP_OK) {
            ESP_LOGE(tag, "Failed to unregister Settings HTTP routes: %s", esp_err_to_name(err));
        }
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_HTTP_SERVER_ENABLE */
        launcher->presentation.settings_http_handler.settings = NULL;
        init_settings_http_routes                             = false;
    }
#endif /* COMPOSITION_MAIN_CONFIG_PRESENTATION_HTTP_SETTINGS_ENABLE */

#ifdef COMPOSITION_MAIN_CONFIG_PRESENTATION_HTTP_NETIF_ENABLE
    if (init_netif_http_routes) {
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_HTTP_SERVER_ENABLE
        esp_err_t err = pres_http_route_netif_unregister(launcher->driver.http_server_handle);
        if (err != ESP_OK) {
            ESP_LOGE(tag, "Failed to unregister Netif HTTP routes: %s", esp_err_to_name(err));
        }
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_HTTP_SERVER_ENABLE */
        launcher->presentation.netif_http_handler.netif = NULL;
        init_netif_http_routes                          = false;
    }
#endif /* COMPOSITION_MAIN_CONFIG_PRESENTATION_HTTP_NETIF_ENABLE */
}
