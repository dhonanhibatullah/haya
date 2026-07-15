#include "infrastructure/messaging/subscribe/stub_impl_utils.h"

#include <string.h>

dom_models_error_t inf_messaging_subscribe_stub_impl_load_cfg(
    inf_messaging_subscribe_stub_impl_ctx_t*       ctx,
    const inf_messaging_subscribe_stub_impl_cfg_t* cfg
) {
    if (!ctx || !cfg) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    memset(ctx, 0, sizeof(inf_messaging_subscribe_stub_impl_ctx_t));

    if (cfg->registration_ack_subscribed) {
        inf_messaging_subscribe_stub_impl_subscribe_registration_ack(ctx);
    }
    if (cfg->update_subscribed) {
        inf_messaging_subscribe_stub_impl_subscribe_update(ctx);
    }
    if (cfg->restart_subscribed) {
        inf_messaging_subscribe_stub_impl_subscribe_restart(ctx);
    }

    ctx->registration_ack_subscribe_cnt = 0;
    ctx->update_subscribe_cnt           = 0;
    ctx->restart_subscribe_cnt          = 0;

    return DOMAIN_MODELS_ERROR_OK;
}

void inf_messaging_subscribe_stub_impl_subscribe_registration_ack(
    inf_messaging_subscribe_stub_impl_ctx_t* ctx
) {
    if (!ctx) {
        return;
    }

    ctx->registration_ack_subscribed = true;
    ctx->registration_ack_subscribe_cnt++;
}

void inf_messaging_subscribe_stub_impl_subscribe_update(
    inf_messaging_subscribe_stub_impl_ctx_t* ctx
) {
    if (!ctx) {
        return;
    }

    ctx->update_subscribed = true;
    ctx->update_subscribe_cnt++;
}

void inf_messaging_subscribe_stub_impl_subscribe_restart(
    inf_messaging_subscribe_stub_impl_ctx_t* ctx
) {
    if (!ctx) {
        return;
    }

    ctx->restart_subscribed = true;
    ctx->restart_subscribe_cnt++;
}
