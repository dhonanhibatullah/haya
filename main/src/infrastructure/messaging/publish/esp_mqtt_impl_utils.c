#include "infrastructure/messaging/publish/esp_mqtt_impl_utils.h"

#include <stdio.h>
#include <string.h>

#include "cJSON.h"
#include "domain/models/error.h"
#include "domain/models/messaging.h"
#include "mqtt_client.h"

/* Helper Function Prototypes */

static bool cstr_available(const char* value);

dom_models_error_t inf_messaging_publish_esp_mqtt_impl_validate_cfg(
    const inf_messaging_publish_esp_mqtt_impl_cfg_t* cfg
) {
    if (!cfg || !cfg->mqtt_client || !cstr_available(cfg->device_id_str) || cfg->qos < 0) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    return DOMAIN_MODELS_ERROR_OK;
}

dom_models_error_t inf_messaging_publish_esp_mqtt_impl_build_device_topic(
    const inf_messaging_publish_esp_mqtt_impl_ctx_t* ctx,
    const char*                                      suffix,
    char*                                            out,
    size_t                                           out_size
) {
    if (!ctx || !cstr_available(ctx->cfg.device_id_str) || !cstr_available(suffix) || !out || out_size == 0) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    int written = snprintf(out, out_size, "/pub/%s/%s", ctx->cfg.device_id_str, suffix);
    if (written <= 0 || (size_t)written >= out_size) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    return DOMAIN_MODELS_ERROR_OK;
}

bool inf_messaging_publish_esp_mqtt_impl_registration_valid(
    const dom_models_messaging_registration_t* registration
) {
    return registration &&
           cstr_available(registration->hardware_mac) &&
           cstr_available(registration->name) &&
           cstr_available(registration->type) &&
           cstr_available(registration->firmware_version) &&
           cstr_available(registration->session_key) &&
           cstr_available(registration->signature);
}

bool inf_messaging_publish_esp_mqtt_impl_status_valid(
    const dom_models_messaging_status_t* status
) {
    return status && cstr_available(status->status);
}

bool inf_messaging_publish_esp_mqtt_impl_log_valid(
    const dom_models_messaging_log_t* log
) {
    return log && cstr_available(log->message);
}

char* inf_messaging_publish_esp_mqtt_impl_build_registration_json(
    const dom_models_messaging_registration_t* registration
) {
    if (!inf_messaging_publish_esp_mqtt_impl_registration_valid(registration)) {
        return NULL;
    }

    cJSON* root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    if (!cJSON_AddStringToObject(root, "hardware_mac", registration->hardware_mac) ||
        !cJSON_AddStringToObject(root, "name", registration->name) ||
        !cJSON_AddStringToObject(root, "type", registration->type) ||
        !cJSON_AddStringToObject(root, "firmware_version", registration->firmware_version) ||
        !cJSON_AddStringToObject(root, "session_key", registration->session_key) ||
        !cJSON_AddStringToObject(root, "signature", registration->signature)) {
        cJSON_Delete(root);
        return NULL;
    }

    char* json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    return json;
}

char* inf_messaging_publish_esp_mqtt_impl_build_status_json(
    const dom_models_messaging_status_t* status
) {
    if (!inf_messaging_publish_esp_mqtt_impl_status_valid(status)) {
        return NULL;
    }

    cJSON* root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    if (!cJSON_AddStringToObject(root, "status", status->status)) {
        cJSON_Delete(root);
        return NULL;
    }

    char* json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    return json;
}

char* inf_messaging_publish_esp_mqtt_impl_build_log_json(
    const dom_models_messaging_log_t* log
) {
    if (!inf_messaging_publish_esp_mqtt_impl_log_valid(log)) {
        return NULL;
    }

    cJSON* root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    if (!cJSON_AddStringToObject(root, "message", log->message)) {
        cJSON_Delete(root);
        return NULL;
    }

    char* json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    return json;
}

dom_models_error_t inf_messaging_publish_esp_mqtt_impl_publish_json(
    const inf_messaging_publish_esp_mqtt_impl_ctx_t* ctx,
    const char*                                      topic,
    char*                                            json,
    bool                                             retained
) {
    if (!ctx || !ctx->cfg.mqtt_client || !cstr_available(topic)) {
        if (json) {
            cJSON_free(json);
        }
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }
    if (!json) {
        return DOMAIN_MODELS_ERROR_MALLOC_FAILED;
    }

    size_t len = strlen(json);
    if (len == 0) {
        cJSON_free(json);
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    int msg_id = esp_mqtt_client_publish(ctx->cfg.mqtt_client, topic, json, (int)len, ctx->cfg.qos, retained ? 1 : 0);
    cJSON_free(json);
    if (msg_id < 0) {
        return DOMAIN_MODELS_ERROR_FAILURE;
    }

    return DOMAIN_MODELS_ERROR_OK;
}

/* Helper Function Implementations */

static bool cstr_available(const char* value) {
    return value && value[0] != '\0';
}
