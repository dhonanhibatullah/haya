#include "infrastructure/messaging/publish/stub_impl_utils.h"

#include <string.h>

/* Helper Function Prototypes */

static bool cstr_available(const char* value);

dom_models_error_t inf_messaging_publish_stub_impl_load_cfg(
    inf_messaging_publish_stub_impl_ctx_t*       ctx,
    const inf_messaging_publish_stub_impl_cfg_t* cfg
) {
    if (!ctx || !cfg) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    memset(ctx, 0, sizeof(inf_messaging_publish_stub_impl_ctx_t));

    if (cfg->registration_available) {
        dom_models_error_t err = inf_messaging_publish_stub_impl_set_registration(ctx, &cfg->registration);
        if (err != DOMAIN_MODELS_ERROR_OK) {
            return err;
        }
    }

    if (cfg->status_available) {
        dom_models_error_t err = inf_messaging_publish_stub_impl_set_status(ctx, &cfg->status);
        if (err != DOMAIN_MODELS_ERROR_OK) {
            return err;
        }
    }

    if (cfg->log_available) {
        dom_models_error_t err = inf_messaging_publish_stub_impl_set_log(ctx, &cfg->log);
        if (err != DOMAIN_MODELS_ERROR_OK) {
            return err;
        }
    }

    ctx->registration_publish_cnt = 0;
    ctx->status_publish_cnt       = 0;
    ctx->log_publish_cnt          = 0;
    ctx->connected                = cfg->connected;

    return DOMAIN_MODELS_ERROR_OK;
}

dom_models_error_t inf_messaging_publish_stub_impl_set_registration(
    inf_messaging_publish_stub_impl_ctx_t*     ctx,
    const dom_models_messaging_registration_t* registration
) {
    if (!ctx || !registration ||
        !cstr_available(registration->hardware_mac) ||
        !cstr_available(registration->name) ||
        !cstr_available(registration->type) ||
        !cstr_available(registration->firmware_version) ||
        !cstr_available(registration->session_key) ||
        !cstr_available(registration->signature)) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    memcpy(&ctx->registration, registration, sizeof(dom_models_messaging_registration_t));
    ctx->registration_available = true;
    ctx->registration_publish_cnt++;

    return DOMAIN_MODELS_ERROR_OK;
}

dom_models_error_t inf_messaging_publish_stub_impl_set_status(
    inf_messaging_publish_stub_impl_ctx_t* ctx,
    const dom_models_messaging_status_t*   status
) {
    if (!ctx || !status || !cstr_available(status->status)) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    memcpy(&ctx->status, status, sizeof(dom_models_messaging_status_t));
    ctx->status_available = true;
    ctx->status_publish_cnt++;

    return DOMAIN_MODELS_ERROR_OK;
}

dom_models_error_t inf_messaging_publish_stub_impl_set_log(
    inf_messaging_publish_stub_impl_ctx_t* ctx,
    const dom_models_messaging_log_t*      log
) {
    if (!ctx || !log || !cstr_available(log->message)) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    memcpy(&ctx->log, log, sizeof(dom_models_messaging_log_t));
    ctx->log_available = true;
    ctx->log_publish_cnt++;

    return DOMAIN_MODELS_ERROR_OK;
}

/* Helper Function Implementations */

static bool cstr_available(const char* value) {
    return value && value[0] != '\0';
}
