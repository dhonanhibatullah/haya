#ifndef INFRASTRUCTURE_MESSAGING_SUBSCRIBE_STUB_IMPL_UTILS_H
#define INFRASTRUCTURE_MESSAGING_SUBSCRIBE_STUB_IMPL_UTILS_H

#include "domain/models/error.h"
#include "infrastructure/messaging/subscribe/stub_impl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

dom_models_error_t inf_messaging_subscribe_stub_impl_load_cfg(
    inf_messaging_subscribe_stub_impl_ctx_t*       ctx,
    const inf_messaging_subscribe_stub_impl_cfg_t* cfg
);

void inf_messaging_subscribe_stub_impl_subscribe_registration_ack(
    inf_messaging_subscribe_stub_impl_ctx_t* ctx
);

void inf_messaging_subscribe_stub_impl_subscribe_update(
    inf_messaging_subscribe_stub_impl_ctx_t* ctx
);

void inf_messaging_subscribe_stub_impl_subscribe_restart(
    inf_messaging_subscribe_stub_impl_ctx_t* ctx
);

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_MESSAGING_SUBSCRIBE_STUB_IMPL_UTILS_H */
