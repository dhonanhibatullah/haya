#ifndef INFRASTRUCTURE_MESSAGING_PUBLISH_ESP_MQTT_IMPL_TYPES_H
#define INFRASTRUCTURE_MESSAGING_PUBLISH_ESP_MQTT_IMPL_TYPES_H

#include <stdbool.h>

#include "mqtt_client.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    esp_mqtt_client_handle_t mqtt_client;
    const char*              device_id_str;
    int                      qos;
    bool                     registration_retained;
    bool                     status_retained;
    bool                     log_retained;
} inf_messaging_publish_esp_mqtt_impl_cfg_t;

#define INF_MESSAGING_PUBLISH_ESP_MQTT_IMPL_CFG_DEFAULT() \
    {                                                     \
        .mqtt_client           = NULL,                    \
        .device_id_str         = NULL,                    \
        .qos                   = 1,                       \
        .registration_retained = false,                   \
        .status_retained       = true,                    \
        .log_retained          = false,                   \
    }

typedef struct {
    inf_messaging_publish_esp_mqtt_impl_cfg_t cfg;
    bool                                      connected;
} inf_messaging_publish_esp_mqtt_impl_ctx_t;

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_MESSAGING_PUBLISH_ESP_MQTT_IMPL_TYPES_H */
