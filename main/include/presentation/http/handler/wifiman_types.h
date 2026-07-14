#ifndef PRESENTATION_HTTP_HANDLER_WIFIMAN_TYPES_H
#define PRESENTATION_HTTP_HANDLER_WIFIMAN_TYPES_H

#include "domain/usecases/wifiman.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    dom_usecases_wifiman_t* wifiman;
} pres_http_handler_wifiman_t;

#ifdef __cplusplus
}
#endif

#endif /* PRESENTATION_HTTP_HANDLER_WIFIMAN_TYPES_H */
