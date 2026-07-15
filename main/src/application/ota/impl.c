#include "application/ota/impl.h"

#include <stdlib.h>
#include <string.h>

#include "application/ota/impl_types.h"
#include "application/ota/impl_utils.h"
#include "domain/models/error.h"
#include "domain/usecases/ota.h"

#define BASE_TAG "ota"

/* Helper Function Prototypes */

static dom_models_error_t get_ctx(
    dom_usecases_ota_t*  self,
    app_ota_impl_ctx_t** out
);

/* Contract Function Prototypes */

static dom_models_error_t update_impl(
    dom_usecases_ota_t*             self,
    const dom_models_update_info_t* update_info
);
static dom_models_error_t validate_impl(
    dom_usecases_ota_t* self
);
static dom_models_error_t rollback_impl(
    dom_usecases_ota_t* self
);
static dom_models_error_t get_status_impl(
    dom_usecases_ota_t*        self,
    dom_usecases_ota_status_t* out
);

/* Constructor and Destructor */

dom_usecases_ota_t* app_ota_impl_new(const app_ota_impl_cfg_t* cfg) {
    const char* tag = BASE_TAG "/new";

    dom_models_error_t err = app_ota_impl_validate_cfg(cfg);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return NULL;
    }

    app_ota_impl_ctx_t* ctx = (app_ota_impl_ctx_t*)calloc(1, sizeof(app_ota_impl_ctx_t));
    if (!ctx) {
        cfg->logger->error(cfg->logger, tag, "Failed to allocate OTA context: %s (%d)", dom_models_error_str(DOMAIN_MODELS_ERROR_MALLOC_FAILED), (int)DOMAIN_MODELS_ERROR_MALLOC_FAILED);
        return NULL;
    }

    memcpy(&ctx->cfg, cfg, sizeof(app_ota_impl_cfg_t));
    ctx->updating = false;

    dom_usecases_ota_t* self = dom_usecases_ota_new(ctx);
    if (!self) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to allocate OTA usecase: %s (%d)", dom_models_error_str(DOMAIN_MODELS_ERROR_MALLOC_FAILED), (int)DOMAIN_MODELS_ERROR_MALLOC_FAILED);
        free(ctx);
        return NULL;
    }

    self->update     = update_impl;
    self->validate   = validate_impl;
    self->rollback   = rollback_impl;
    self->get_status = get_status_impl;

    ctx->cfg.logger->info(ctx->cfg.logger, tag, "OTA created successfully");

    return self;
}

void app_ota_impl_delete(dom_usecases_ota_t* self) {
    const char* tag = BASE_TAG "/delete";

    if (!self) {
        return;
    }

    app_ota_impl_ctx_t* ctx = self->ctx;
    if (ctx) {
        ctx->cfg.logger->info(ctx->cfg.logger, tag, "OTA deleted successfully");
        free(ctx);
    }

    dom_usecases_ota_delete(self);
}

/* Contract Function Implementations */

static dom_models_error_t update_impl(
    dom_usecases_ota_t*             self,
    const dom_models_update_info_t* update_info
) {
    const char* tag = BASE_TAG "/update";

    app_ota_impl_ctx_t* ctx = NULL;
    dom_models_error_t  err = get_ctx(self, &ctx);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    err = app_ota_impl_validate_update_info(update_info);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Invalid update info: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    if (ctx->updating) {
        err = DOMAIN_MODELS_ERROR_BAD_STATE;
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "OTA update already in progress: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    ctx->updating = true;
    ctx->cfg.logger->info(ctx->cfg.logger, tag, "Starting OTA update from URL: %s", update_info->firmware_url);

    err = ctx->cfg.update->update(ctx->cfg.update, update_info);
    ctx->updating = false;

    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "OTA update failed: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    ctx->cfg.logger->info(ctx->cfg.logger, tag, "OTA update completed successfully");
    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t validate_impl(
    dom_usecases_ota_t* self
) {
    const char* tag = BASE_TAG "/validate";

    app_ota_impl_ctx_t* ctx = NULL;
    dom_models_error_t  err = get_ctx(self, &ctx);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    ctx->cfg.logger->info(ctx->cfg.logger, tag, "Validating running application partition");

    err = ctx->cfg.update->validate(ctx->cfg.update);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to validate running partition: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    ctx->cfg.logger->info(ctx->cfg.logger, tag, "Running partition validated successfully");
    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t rollback_impl(
    dom_usecases_ota_t* self
) {
    const char* tag = BASE_TAG "/rollback";

    app_ota_impl_ctx_t* ctx = NULL;
    dom_models_error_t  err = get_ctx(self, &ctx);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    ctx->cfg.logger->info(ctx->cfg.logger, tag, "Requesting rollback to previous partition");

    err = ctx->cfg.update->rollback(ctx->cfg.update);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to perform rollback: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    ctx->cfg.logger->info(ctx->cfg.logger, tag, "Rollback call finished");
    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t get_status_impl(
    dom_usecases_ota_t*        self,
    dom_usecases_ota_status_t* out
) {
    const char* tag = BASE_TAG "/get_status";

    app_ota_impl_ctx_t* ctx = NULL;
    dom_models_error_t  err = get_ctx(self, &ctx);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    if (!out) {
        err = DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Missing OTA status output pointer: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    out->updating = ctx->updating;
    return DOMAIN_MODELS_ERROR_OK;
}

/* Helper Function Implementations */

static dom_models_error_t get_ctx(
    dom_usecases_ota_t*  self,
    app_ota_impl_ctx_t** out
) {
    if (!self) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    app_ota_impl_ctx_t* ctx = (app_ota_impl_ctx_t*)self->ctx;
    if (!ctx) {
        return DOMAIN_MODELS_ERROR_BAD_STATE;
    }

    if (!out) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    *out = ctx;
    return DOMAIN_MODELS_ERROR_OK;
}
