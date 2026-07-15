#ifndef COMPOSITION_MAIN_UTILS_H
#define COMPOSITION_MAIN_UTILS_H

#include <stdbool.h>

#include "composition/main/types.h"
#include "domain/models/error.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_MQTT_CLIENT_ENABLE
bool cmp_main_utils_cstr_available(const char* value);

dom_models_error_t cmp_main_utils_mqtt_prepare_runtime(cmp_main_launcher_t* launcher);

dom_models_error_t cmp_main_utils_mqtt_error_from_esp(esp_err_t err);

void cmp_main_utils_mqtt_clear_runtime(cmp_main_launcher_t* launcher);
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_MQTT_CLIENT_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /* COMPOSITION_MAIN_UTILS_H */
