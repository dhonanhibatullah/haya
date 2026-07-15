#ifndef INFRASTRUCTURE_MESSAGING_SUBSCRIBE_ESP_MQTT_IMPL_UTILS_H
#define INFRASTRUCTURE_MESSAGING_SUBSCRIBE_ESP_MQTT_IMPL_UTILS_H

#include <stddef.h>

#include "domain/models/error.h"
#include "infrastructure/messaging/subscribe/esp_mqtt_impl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

dom_models_error_t inf_messaging_subscribe_esp_mqtt_impl_validate_cfg(
    const inf_messaging_subscribe_esp_mqtt_impl_cfg_t* cfg
);

dom_models_error_t inf_messaging_subscribe_esp_mqtt_impl_build_topic(
    const inf_messaging_subscribe_esp_mqtt_impl_ctx_t* ctx,
    const char*                                        suffix,
    char*                                              out,
    size_t                                             out_size
);

dom_models_error_t inf_messaging_subscribe_esp_mqtt_impl_subscribe_suffix(
    const inf_messaging_subscribe_esp_mqtt_impl_ctx_t* ctx,
    const char*                                        suffix
);

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_MESSAGING_SUBSCRIBE_ESP_MQTT_IMPL_UTILS_H */
