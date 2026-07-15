#ifndef INFRASTRUCTURE_MESSAGING_PUBLISH_STUB_IMPL_UTILS_H
#define INFRASTRUCTURE_MESSAGING_PUBLISH_STUB_IMPL_UTILS_H

#include "domain/models/error.h"
#include "domain/models/messaging.h"
#include "infrastructure/messaging/publish/stub_impl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

dom_models_error_t inf_messaging_publish_stub_impl_load_cfg(
    inf_messaging_publish_stub_impl_ctx_t*       ctx,
    const inf_messaging_publish_stub_impl_cfg_t* cfg
);

dom_models_error_t inf_messaging_publish_stub_impl_set_registration(
    inf_messaging_publish_stub_impl_ctx_t*     ctx,
    const dom_models_messaging_registration_t* registration
);

dom_models_error_t inf_messaging_publish_stub_impl_set_status(
    inf_messaging_publish_stub_impl_ctx_t* ctx,
    const dom_models_messaging_status_t*   status
);

dom_models_error_t inf_messaging_publish_stub_impl_set_log(
    inf_messaging_publish_stub_impl_ctx_t* ctx,
    const dom_models_messaging_log_t*      log
);

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_MESSAGING_PUBLISH_STUB_IMPL_UTILS_H */
