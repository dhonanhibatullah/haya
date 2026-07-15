#ifndef PRESENTATION_HTTP_HANDLER_SETTINGS_TYPES_H
#define PRESENTATION_HTTP_HANDLER_SETTINGS_TYPES_H

#include "domain/usecases/settings.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    dom_usecases_settings_t* settings;
} pres_http_handler_settings_t;

#ifdef __cplusplus
}
#endif

#endif /* PRESENTATION_HTTP_HANDLER_SETTINGS_TYPES_H */
