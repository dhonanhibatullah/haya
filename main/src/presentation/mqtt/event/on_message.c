#include "presentation/mqtt/event/on_message.h"
#include <stdio.h>
#include <string.h>
#include "presentation/mqtt/handler/reset.h"
#include "presentation/mqtt/handler/ota.h"

#define TAG "pres_mqtt_on_message"

void pres_mqtt_event_on_message(pres_mqtt_context_t* ctx, esp_mqtt_event_handle_t event) {
    if (event->topic_len <= 0 || event->data_len < 0) {
        return;
    }

    char topic[128];
    if (event->topic_len >= sizeof(topic)) {
        ctx->logger->error(ctx->logger, TAG, "Topic too long");
        return;
    }
    memcpy(topic, event->topic, event->topic_len);
    topic[event->topic_len] = '\0';

    char reset_expected[64];
    char ota_expected[64];
    snprintf(reset_expected, sizeof(reset_expected), "/sub/%s/reset", ctx->device_id_str);
    snprintf(ota_expected, sizeof(ota_expected), "/sub/%s/ota", ctx->device_id_str);

    if (strcmp(topic, reset_expected) == 0) {
        pres_mqtt_handler_reset(ctx, event->data, event->data_len);
    } else if (strcmp(topic, ota_expected) == 0) {
        pres_mqtt_handler_ota(ctx, event->data, event->data_len);
    } else {
        ctx->logger->debug(ctx->logger, TAG, "Unhandled topic: %s", topic);
    }
}
