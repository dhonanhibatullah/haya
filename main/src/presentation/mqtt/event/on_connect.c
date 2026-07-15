#include "presentation/mqtt/event/on_connect.h"
#include <stdio.h>
#include "mqtt_client.h"

#define TAG "pres_mqtt_on_connect"

void pres_mqtt_event_on_connect(pres_mqtt_context_t* ctx, esp_mqtt_event_handle_t event) {
    ctx->logger->info(ctx->logger, TAG, "Connected to MQTT broker");

    char topic[128];

    // Reset Subscription
    snprintf(topic, sizeof(topic), "/sub/%s/reset", ctx->device_id_str);
    int msg_id = esp_mqtt_client_subscribe(event->client, topic, 1);
    ctx->logger->info(ctx->logger, TAG, "Subscribed to reset: %s (msg_id=%d)", topic, msg_id);

    // OTA Subscription
    snprintf(topic, sizeof(topic), "/sub/%s/ota", ctx->device_id_str);
    msg_id = esp_mqtt_client_subscribe(event->client, topic, 1);
    ctx->logger->info(ctx->logger, TAG, "Subscribed to ota: %s (msg_id=%d)", topic, msg_id);
}
