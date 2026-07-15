#include "presentation/mqtt/handler/reset.h"

#define TAG "pres_mqtt_reset"

void pres_mqtt_handler_reset(pres_mqtt_context_t* ctx, const char* data, int data_len) {
    ctx->logger->info(ctx->logger, TAG, "Received reset request via MQTT");
    dom_models_error_t err = ctx->settings->restart(ctx->settings, 0);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->logger->error(ctx->logger, TAG, "Failed to execute restart: %s (%d)", dom_models_error_str(err), (int)err);
    }
}
