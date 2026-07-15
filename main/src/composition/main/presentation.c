#include "composition/main/presentation.h"

#include "composition/main/config.h"                       // IWYU pragma: keep
#include "domain/models/error.h"                           // IWYU pragma: keep
#include "esp_err.h"                                       // IWYU pragma: keep
#include "esp_log.h"                                       // IWYU pragma: keep
#include "presentation/http/route/wifiman.h"               // IWYU pragma: keep
#include "presentation/task/wifiman_sta_reconnect/task.h"  // IWYU pragma: keep

#define TAG_PATH "main/presentation"

/* Init Flags for Deinitizalization Sequence */

static bool init_wifiman_http_routes        = false;
static bool init_wifiman_sta_reconnect_task = false;

dom_models_error_t cmp_main_presentation_init(cmp_main_launcher_t* launcher) {
    const char* tag = TAG_PATH "/init";

    if (!launcher) {
        ESP_LOGE(tag, "Invalid launcher");
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

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

    return DOMAIN_MODELS_ERROR_OK;
}

void cmp_main_presentation_deinit(cmp_main_launcher_t* launcher) {
    const char* tag = TAG_PATH "/deinit";

    if (!launcher) {
        return;
    }

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
}
