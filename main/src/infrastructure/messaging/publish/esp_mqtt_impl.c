#include "infrastructure/messaging/publish/esp_mqtt_impl.h"

#include <stdlib.h>
#include <string.h>

#include "domain/contracts/messaging/publish.h"
#include "domain/models/error.h"
#include "domain/models/messaging.h"
#include "infrastructure/messaging/publish/esp_mqtt_impl_utils.h"

/* Contract Function Prototypes */

static dom_models_error_t send_registration_impl(
    dom_contracts_messaging_publish_t*         self,
    const dom_models_messaging_registration_t* registration
);
static dom_models_error_t send_status_impl(
    dom_contracts_messaging_publish_t*   self,
    const dom_models_messaging_status_t* status
);
static dom_models_error_t send_log_impl(
    dom_contracts_messaging_publish_t* self,
    const dom_models_messaging_log_t*  log
);
static dom_models_error_t is_connected_impl(
    dom_contracts_messaging_publish_t* self,
    bool*                              out
);

/* Event Handler for connection tracking */
static void esp_mqtt_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data) {
    inf_messaging_publish_esp_mqtt_impl_ctx_t* ctx = (inf_messaging_publish_esp_mqtt_impl_ctx_t*)handler_args;
    if (!ctx) return;
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ctx->connected = true;
            break;
        case MQTT_EVENT_DISCONNECTED:
            ctx->connected = false;
            break;
        default:
            break;
    }
}

/* Constructor and Destructor */

dom_contracts_messaging_publish_t* inf_messaging_publish_esp_mqtt_impl_new(
    const inf_messaging_publish_esp_mqtt_impl_cfg_t* cfg
) {
    inf_messaging_publish_esp_mqtt_impl_ctx_t* ctx = (inf_messaging_publish_esp_mqtt_impl_ctx_t*)calloc(1, sizeof(inf_messaging_publish_esp_mqtt_impl_ctx_t));
    if (!ctx) {
        return NULL;
    }

    inf_messaging_publish_esp_mqtt_impl_cfg_t default_cfg = INF_MESSAGING_PUBLISH_ESP_MQTT_IMPL_CFG_DEFAULT();
    memcpy(&ctx->cfg, cfg ? cfg : &default_cfg, sizeof(inf_messaging_publish_esp_mqtt_impl_cfg_t));

    dom_models_error_t err = inf_messaging_publish_esp_mqtt_impl_validate_cfg(&ctx->cfg);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        free(ctx);
        return NULL;
    }

    dom_contracts_messaging_publish_t* self = dom_contracts_messaging_publish_new(ctx);
    if (!self) {
        free(ctx);
        return NULL;
    }

    self->send_registration = send_registration_impl;
    self->send_status       = send_status_impl;
    self->send_log          = send_log_impl;
    self->is_connected      = is_connected_impl;

    esp_err_t event_err = esp_mqtt_client_register_event(
        ctx->cfg.mqtt_client,
        ESP_EVENT_ANY_ID,
        esp_mqtt_event_handler,
        ctx
    );
    if (event_err != ESP_OK) {
        dom_contracts_messaging_publish_delete(self);
        free(ctx);
        return NULL;
    }

    return self;
}

void inf_messaging_publish_esp_mqtt_impl_delete(dom_contracts_messaging_publish_t* self) {
    if (!self) {
        return;
    }

    inf_messaging_publish_esp_mqtt_impl_ctx_t* ctx = self->ctx;
    if (ctx) {
        if (ctx->cfg.mqtt_client) {
            esp_mqtt_client_unregister_event(
                ctx->cfg.mqtt_client,
                ESP_EVENT_ANY_ID,
                esp_mqtt_event_handler
            );
        }
        free(ctx);
    }
    dom_contracts_messaging_publish_delete(self);
}

/* Contract Function Implementations */

static dom_models_error_t send_registration_impl(
    dom_contracts_messaging_publish_t*         self,
    const dom_models_messaging_registration_t* registration
) {
    if (!self || !self->ctx || !registration) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }
    if (!inf_messaging_publish_esp_mqtt_impl_registration_valid(registration)) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_messaging_publish_esp_mqtt_impl_ctx_t* ctx = self->ctx;

    return inf_messaging_publish_esp_mqtt_impl_publish_json(
        ctx,
        "/pub/reg",
        inf_messaging_publish_esp_mqtt_impl_build_registration_json(registration),
        ctx->cfg.registration_retained
    );
}

static dom_models_error_t send_status_impl(
    dom_contracts_messaging_publish_t*   self,
    const dom_models_messaging_status_t* status
) {
    if (!self || !self->ctx || !status) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }
    if (!inf_messaging_publish_esp_mqtt_impl_status_valid(status)) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_messaging_publish_esp_mqtt_impl_ctx_t* ctx = self->ctx;

    char               topic[DOM_MODELS_MESSAGING_TOPIC_MAX_LEN];
    dom_models_error_t err = inf_messaging_publish_esp_mqtt_impl_build_device_topic(ctx, "status", topic, sizeof(topic));
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    return inf_messaging_publish_esp_mqtt_impl_publish_json(
        ctx,
        topic,
        inf_messaging_publish_esp_mqtt_impl_build_status_json(status),
        ctx->cfg.status_retained
    );
}

static dom_models_error_t send_log_impl(
    dom_contracts_messaging_publish_t* self,
    const dom_models_messaging_log_t*  log
) {
    if (!self || !self->ctx || !log) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }
    if (!inf_messaging_publish_esp_mqtt_impl_log_valid(log)) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_messaging_publish_esp_mqtt_impl_ctx_t* ctx = self->ctx;

    char               topic[DOM_MODELS_MESSAGING_TOPIC_MAX_LEN];
    dom_models_error_t err = inf_messaging_publish_esp_mqtt_impl_build_device_topic(ctx, "log", topic, sizeof(topic));
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    return inf_messaging_publish_esp_mqtt_impl_publish_json(
        ctx,
        topic,
        inf_messaging_publish_esp_mqtt_impl_build_log_json(log),
        ctx->cfg.log_retained
    );
}

static dom_models_error_t is_connected_impl(
    dom_contracts_messaging_publish_t* self,
    bool*                              out
) {
    if (!self || !self->ctx || !out) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_messaging_publish_esp_mqtt_impl_ctx_t* ctx = self->ctx;
    *out                                           = ctx->connected;

    return DOMAIN_MODELS_ERROR_OK;
}
