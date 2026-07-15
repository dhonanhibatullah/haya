#include "infrastructure/messaging/subscribe/esp_mqtt_impl.h"

#include <stdlib.h>
#include <string.h>

#include "domain/contracts/messaging/subscribe.h"
#include "domain/models/error.h"
#include "infrastructure/messaging/subscribe/esp_mqtt_impl_utils.h"

/* Contract Function Prototypes */

static dom_models_error_t subscribe_registration_ack_impl(
    dom_contracts_messaging_subscribe_t* self
);
static dom_models_error_t subscribe_update_impl(
    dom_contracts_messaging_subscribe_t* self
);
static dom_models_error_t subscribe_restart_impl(
    dom_contracts_messaging_subscribe_t* self
);

/* Constructor and Destructor */

dom_contracts_messaging_subscribe_t* inf_messaging_subscribe_esp_mqtt_impl_new(
    const inf_messaging_subscribe_esp_mqtt_impl_cfg_t* cfg
) {
    inf_messaging_subscribe_esp_mqtt_impl_ctx_t* ctx = (inf_messaging_subscribe_esp_mqtt_impl_ctx_t*)calloc(1, sizeof(inf_messaging_subscribe_esp_mqtt_impl_ctx_t));
    if (!ctx) {
        return NULL;
    }

    inf_messaging_subscribe_esp_mqtt_impl_cfg_t default_cfg = INF_MESSAGING_SUBSCRIBE_ESP_MQTT_IMPL_CFG_DEFAULT();
    memcpy(&ctx->cfg, cfg ? cfg : &default_cfg, sizeof(inf_messaging_subscribe_esp_mqtt_impl_cfg_t));

    dom_models_error_t err = inf_messaging_subscribe_esp_mqtt_impl_validate_cfg(&ctx->cfg);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        free(ctx);
        return NULL;
    }

    dom_contracts_messaging_subscribe_t* self = dom_contracts_messaging_subscribe_new(ctx);
    if (!self) {
        free(ctx);
        return NULL;
    }

    self->subscribe_registration_ack = subscribe_registration_ack_impl;
    self->subscribe_update           = subscribe_update_impl;
    self->subscribe_restart          = subscribe_restart_impl;

    return self;
}

void inf_messaging_subscribe_esp_mqtt_impl_delete(dom_contracts_messaging_subscribe_t* self) {
    if (!self) {
        return;
    }

    free(self->ctx);
    dom_contracts_messaging_subscribe_delete(self);
}

/* Contract Function Implementations */

static dom_models_error_t subscribe_registration_ack_impl(
    dom_contracts_messaging_subscribe_t* self
) {
    if (!self || !self->ctx) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    return inf_messaging_subscribe_esp_mqtt_impl_subscribe_suffix(self->ctx, "reg_ack");
}

static dom_models_error_t subscribe_update_impl(
    dom_contracts_messaging_subscribe_t* self
) {
    if (!self || !self->ctx) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    return inf_messaging_subscribe_esp_mqtt_impl_subscribe_suffix(self->ctx, "update");
}

static dom_models_error_t subscribe_restart_impl(
    dom_contracts_messaging_subscribe_t* self
) {
    if (!self || !self->ctx) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    return inf_messaging_subscribe_esp_mqtt_impl_subscribe_suffix(self->ctx, "restart");
}
