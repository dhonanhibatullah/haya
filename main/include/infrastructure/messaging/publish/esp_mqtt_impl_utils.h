#ifndef INFRASTRUCTURE_MESSAGING_PUBLISH_ESP_MQTT_IMPL_UTILS_H
#define INFRASTRUCTURE_MESSAGING_PUBLISH_ESP_MQTT_IMPL_UTILS_H

#include <stdbool.h>
#include <stddef.h>

#include "domain/models/error.h"
#include "domain/models/messaging.h"
#include "infrastructure/messaging/publish/esp_mqtt_impl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

dom_models_error_t inf_messaging_publish_esp_mqtt_impl_validate_cfg(
    const inf_messaging_publish_esp_mqtt_impl_cfg_t* cfg
);

dom_models_error_t inf_messaging_publish_esp_mqtt_impl_build_device_topic(
    const inf_messaging_publish_esp_mqtt_impl_ctx_t* ctx,
    const char*                                      suffix,
    char*                                            out,
    size_t                                           out_size
);

bool inf_messaging_publish_esp_mqtt_impl_registration_valid(
    const dom_models_messaging_registration_t* registration
);

bool inf_messaging_publish_esp_mqtt_impl_status_valid(
    const dom_models_messaging_status_t* status
);

bool inf_messaging_publish_esp_mqtt_impl_log_valid(
    const dom_models_messaging_log_t* log
);

char* inf_messaging_publish_esp_mqtt_impl_build_registration_json(
    const dom_models_messaging_registration_t* registration
);

char* inf_messaging_publish_esp_mqtt_impl_build_status_json(
    const dom_models_messaging_status_t* status
);

char* inf_messaging_publish_esp_mqtt_impl_build_log_json(
    const dom_models_messaging_log_t* log
);

dom_models_error_t inf_messaging_publish_esp_mqtt_impl_publish_json(
    const inf_messaging_publish_esp_mqtt_impl_ctx_t* ctx,
    const char*                                      topic,
    char*                                            json,
    bool                                             retained
);

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_MESSAGING_PUBLISH_ESP_MQTT_IMPL_UTILS_H */
