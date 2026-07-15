#include "infrastructure/messaging/publish/stub_impl.h"

#include <stdlib.h>

#include "domain/contracts/messaging/publish.h"
#include "domain/models/error.h"
#include "domain/models/messaging.h"
#include "infrastructure/messaging/publish/stub_impl_utils.h"

/* Contract Function Prototypes */

static dom_models_error_t send_registration_impl(
    dom_contracts_messaging_publish_t*         self,
    const dom_models_messaging_registration_t* registration
);
static dom_models_error_t send_status_impl(
    dom_contracts_messaging_publish_t*   self,
    const dom_models_messaging_status_t* status
);
static dom_models_error_t send_log_impl(
    dom_contracts_messaging_publish_t* self,
    const dom_models_messaging_log_t*  log
);
static dom_models_error_t is_connected_impl(
    dom_contracts_messaging_publish_t* self,
    bool*                              out
);

/* Constructor and Destructor */

dom_contracts_messaging_publish_t* inf_messaging_publish_stub_impl_new(
    const inf_messaging_publish_stub_impl_cfg_t* cfg
) {
    inf_messaging_publish_stub_impl_ctx_t* ctx = (inf_messaging_publish_stub_impl_ctx_t*)calloc(1, sizeof(inf_messaging_publish_stub_impl_ctx_t));
    if (!ctx) {
        return NULL;
    }

    inf_messaging_publish_stub_impl_cfg_t default_cfg = INF_MESSAGING_PUBLISH_STUB_IMPL_CFG_DEFAULT();
    dom_models_error_t                    err         = inf_messaging_publish_stub_impl_load_cfg(ctx, cfg ? cfg : &default_cfg);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        free(ctx);
        return NULL;
    }

    dom_contracts_messaging_publish_t* self = dom_contracts_messaging_publish_new(ctx);
    if (!self) {
        free(ctx);
        return NULL;
    }

    self->send_registration = send_registration_impl;
    self->send_status       = send_status_impl;
    self->send_log          = send_log_impl;
    self->is_connected      = is_connected_impl;

    return self;
}

void inf_messaging_publish_stub_impl_delete(dom_contracts_messaging_publish_t* self) {
    if (!self) {
        return;
    }

    free(self->ctx);
    dom_contracts_messaging_publish_delete(self);
}

/* Contract Function Implementations */

static dom_models_error_t send_registration_impl(
    dom_contracts_messaging_publish_t*         self,
    const dom_models_messaging_registration_t* registration
) {
    if (!self || !self->ctx) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    return inf_messaging_publish_stub_impl_set_registration(self->ctx, registration);
}

static dom_models_error_t send_status_impl(
    dom_contracts_messaging_publish_t*   self,
    const dom_models_messaging_status_t* status
) {
    if (!self || !self->ctx) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    return inf_messaging_publish_stub_impl_set_status(self->ctx, status);
}

static dom_models_error_t send_log_impl(
    dom_contracts_messaging_publish_t* self,
    const dom_models_messaging_log_t*  log
) {
    if (!self || !self->ctx) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    return inf_messaging_publish_stub_impl_set_log(self->ctx, log);
}

static dom_models_error_t is_connected_impl(
    dom_contracts_messaging_publish_t* self,
    bool*                              out
) {
    if (!self || !self->ctx || !out) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_messaging_publish_stub_impl_ctx_t* ctx = self->ctx;
    *out                                         = ctx->connected;

    return DOMAIN_MODELS_ERROR_OK;
}
