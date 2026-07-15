#ifndef INFRASTRUCTURE_MESSAGING_PUBLISH_STUB_IMPL_TYPES_H
#define INFRASTRUCTURE_MESSAGING_PUBLISH_STUB_IMPL_TYPES_H

#include <stdbool.h>
#include <stddef.h>

#include "domain/models/messaging.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool                                registration_available;
    bool                                status_available;
    bool                                log_available;
    dom_models_messaging_registration_t registration;
    dom_models_messaging_status_t       status;
    dom_models_messaging_log_t          log;
} inf_messaging_publish_stub_impl_cfg_t;

#define INF_MESSAGING_PUBLISH_STUB_IMPL_CFG_DEFAULT() \
    {                                                 \
        .registration_available = false,              \
        .status_available       = false,              \
        .log_available          = false,              \
    }

typedef struct {
    bool                                registration_available;
    bool                                status_available;
    bool                                log_available;
    dom_models_messaging_registration_t registration;
    dom_models_messaging_status_t       status;
    dom_models_messaging_log_t          log;
    size_t                              registration_publish_cnt;
    size_t                              status_publish_cnt;
    size_t                              log_publish_cnt;
} inf_messaging_publish_stub_impl_ctx_t;

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_MESSAGING_PUBLISH_STUB_IMPL_TYPES_H */
