#ifndef INFRASTRUCTURE_MESSAGING_SUBSCRIBE_ESP_MQTT_IMPL_TYPES_H
#define INFRASTRUCTURE_MESSAGING_SUBSCRIBE_ESP_MQTT_IMPL_TYPES_H

#include "mqtt_client.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    esp_mqtt_client_handle_t mqtt_client;
    const char*              device_id_str;
    int                      qos;
} inf_messaging_subscribe_esp_mqtt_impl_cfg_t;

#define INF_MESSAGING_SUBSCRIBE_ESP_MQTT_IMPL_CFG_DEFAULT() \
    {                                                       \
        .mqtt_client   = NULL,                              \
        .device_id_str = NULL,                              \
        .qos           = 1,                                 \
    }

typedef struct {
    inf_messaging_subscribe_esp_mqtt_impl_cfg_t cfg;
} inf_messaging_subscribe_esp_mqtt_impl_ctx_t;

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_MESSAGING_SUBSCRIBE_ESP_MQTT_IMPL_TYPES_H */
