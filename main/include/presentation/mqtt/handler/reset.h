#ifndef PRESENTATION_MQTT_HANDLER_RESET_H
#define PRESENTATION_MQTT_HANDLER_RESET_H

#include <stddef.h>

#include "presentation/mqtt/context.h"

#ifdef __cplusplus
extern "C" {
#endif

void pres_mqtt_handler_reset(pres_mqtt_context_t* ctx, const char* data, int data_len);

#ifdef __cplusplus
}
#endif

#endif /* PRESENTATION_MQTT_HANDLER_RESET_H */
