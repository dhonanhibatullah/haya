#include "composition/main/utils.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "composition/main/config.h"
#include "domain/models/error.h"
#include "domain/models/preloaded.h"
#include "esp_err.h"

#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_MQTT_CLIENT_ENABLE

/* Helper Function Prototypes */

static dom_models_error_t mqtt_alloc_format(char** out, const char* format, ...);

/* Implementations */

bool cmp_main_utils_cstr_available(const char* value) {
    return value && value[0] != '\0';
}

dom_models_error_t cmp_main_utils_mqtt_prepare_runtime(cmp_main_launcher_t* launcher) {
    if (!launcher) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }
    if (!cmp_main_utils_cstr_available(dom_models_preloaded_data.mqtt_proto) ||
        !cmp_main_utils_cstr_available(dom_models_preloaded_data.mqtt_host) ||
        !cmp_main_utils_cstr_available(dom_models_preloaded_data.mqtt_port) ||
        !cmp_main_utils_cstr_available(dom_models_preloaded_data.device_id_str) ||
        !cmp_main_config.driver.mqtt_client_lwt_msg ||
        cmp_main_config.driver.mqtt_client_lwt_qos < 0 ||
        cmp_main_config.driver.mqtt_client_lwt_qos > 2 ||
        cmp_main_config.driver.mqtt_client_reconnect_timeout_ms <= 0) {
        return DOMAIN_MODELS_ERROR_BAD_STATE;
    }

    cmp_main_utils_mqtt_clear_runtime(launcher);

    dom_models_error_t err = mqtt_alloc_format(
        &launcher->driver.mqtt_client_url,
        "%s://%s:%s",
        dom_models_preloaded_data.mqtt_proto,
        dom_models_preloaded_data.mqtt_host,
        dom_models_preloaded_data.mqtt_port
    );
    if (err != DOMAIN_MODELS_ERROR_OK) {
        cmp_main_utils_mqtt_clear_runtime(launcher);
        return err;
    }

    err = mqtt_alloc_format(
        &launcher->driver.mqtt_client_id,
        "%s-%s",
        PROJECT_NAME,
        dom_models_preloaded_data.device_id_str
    );
    if (err != DOMAIN_MODELS_ERROR_OK) {
        cmp_main_utils_mqtt_clear_runtime(launcher);
        return err;
    }

    err = mqtt_alloc_format(
        &launcher->driver.mqtt_client_lwt_topic,
        "/pub/%s/status",
        dom_models_preloaded_data.device_id_str
    );
    if (err != DOMAIN_MODELS_ERROR_OK) {
        cmp_main_utils_mqtt_clear_runtime(launcher);
    }

    return err;
}

dom_models_error_t cmp_main_utils_mqtt_error_from_esp(esp_err_t err) {
    switch (err) {
        case ESP_OK:
            return DOMAIN_MODELS_ERROR_OK;
        case ESP_ERR_NO_MEM:
            return DOMAIN_MODELS_ERROR_MALLOC_FAILED;
        case ESP_ERR_INVALID_ARG:
            return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
        case ESP_ERR_INVALID_STATE:
            return DOMAIN_MODELS_ERROR_BAD_STATE;
        case ESP_ERR_TIMEOUT:
            return DOMAIN_MODELS_ERROR_TIMEOUT;
        case ESP_ERR_NOT_SUPPORTED:
            return DOMAIN_MODELS_ERROR_NOT_SUPPORTED;
        default:
            return DOMAIN_MODELS_ERROR_FAILURE;
    }
}

void cmp_main_utils_mqtt_clear_runtime(cmp_main_launcher_t* launcher) {
    if (!launcher) {
        return;
    }

    free(launcher->driver.mqtt_client_url);
    free(launcher->driver.mqtt_client_id);
    free(launcher->driver.mqtt_client_lwt_topic);

    launcher->driver.mqtt_client_url       = NULL;
    launcher->driver.mqtt_client_id        = NULL;
    launcher->driver.mqtt_client_lwt_topic = NULL;
}

/* Helper Function Implementations */

static dom_models_error_t mqtt_alloc_format(char** out, const char* format, ...) {
    if (!out || !format) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    va_list args;
    va_start(args, format);
    int needed = vsnprintf(NULL, 0, format, args);
    va_end(args);
    if (needed < 0) {
        return DOMAIN_MODELS_ERROR_FAILURE;
    }

    char* next = (char*)malloc((size_t)needed + 1);
    if (!next) {
        return DOMAIN_MODELS_ERROR_MALLOC_FAILED;
    }

    va_start(args, format);
    int written = vsnprintf(next, (size_t)needed + 1, format, args);
    va_end(args);
    if (written != needed) {
        free(next);
        return DOMAIN_MODELS_ERROR_FAILURE;
    }

    free(*out);
    *out = next;

    return DOMAIN_MODELS_ERROR_OK;
}

#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_MQTT_CLIENT_ENABLE */
