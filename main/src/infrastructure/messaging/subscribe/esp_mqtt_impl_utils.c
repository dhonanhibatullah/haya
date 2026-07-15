#include "infrastructure/messaging/subscribe/esp_mqtt_impl_utils.h"

#include <stdbool.h>
#include <stdio.h>

#include "domain/models/error.h"
#include "domain/models/messaging.h"
#include "mqtt_client.h"

/* Helper Function Prototypes */

static bool cstr_available(const char* value);

dom_models_error_t inf_messaging_subscribe_esp_mqtt_impl_validate_cfg(
    const inf_messaging_subscribe_esp_mqtt_impl_cfg_t* cfg
) {
    if (!cfg || !cfg->mqtt_client || !cstr_available(cfg->device_id_str) || cfg->qos < 0) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    return DOMAIN_MODELS_ERROR_OK;
}

dom_models_error_t inf_messaging_subscribe_esp_mqtt_impl_build_topic(
    const inf_messaging_subscribe_esp_mqtt_impl_ctx_t* ctx,
    const char*                                        suffix,
    char*                                              out,
    size_t                                             out_size
) {
    if (!ctx || !cstr_available(ctx->cfg.device_id_str) || !cstr_available(suffix) || !out || out_size == 0) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    int written = snprintf(out, out_size, "/sub/%s/%s", ctx->cfg.device_id_str, suffix);
    if (written <= 0 || (size_t)written >= out_size) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    return DOMAIN_MODELS_ERROR_OK;
}

dom_models_error_t inf_messaging_subscribe_esp_mqtt_impl_subscribe_suffix(
    const inf_messaging_subscribe_esp_mqtt_impl_ctx_t* ctx,
    const char*                                        suffix
) {
    if (!ctx || !ctx->cfg.mqtt_client) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    char               topic[DOM_MODELS_MESSAGING_TOPIC_MAX_LEN];
    dom_models_error_t err = inf_messaging_subscribe_esp_mqtt_impl_build_topic(ctx, suffix, topic, sizeof(topic));
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    int msg_id = esp_mqtt_client_subscribe_single(ctx->cfg.mqtt_client, topic, ctx->cfg.qos);
    if (msg_id < 0) {
        return DOMAIN_MODELS_ERROR_FAILURE;
    }

    return DOMAIN_MODELS_ERROR_OK;
}

/* Helper Function Implementations */

static bool cstr_available(const char* value) {
    return value && value[0] != '\0';
}
