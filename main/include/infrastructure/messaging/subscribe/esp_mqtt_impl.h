#ifndef INFRASTRUCTURE_MESSAGING_SUBSCRIBE_ESP_MQTT_IMPL_H
#define INFRASTRUCTURE_MESSAGING_SUBSCRIBE_ESP_MQTT_IMPL_H

#include "domain/contracts/messaging/subscribe.h"
#include "infrastructure/messaging/subscribe/esp_mqtt_impl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

dom_contracts_messaging_subscribe_t* inf_messaging_subscribe_esp_mqtt_impl_new(
    const inf_messaging_subscribe_esp_mqtt_impl_cfg_t* cfg
);

void inf_messaging_subscribe_esp_mqtt_impl_delete(dom_contracts_messaging_subscribe_t* self);

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_MESSAGING_SUBSCRIBE_ESP_MQTT_IMPL_H */
