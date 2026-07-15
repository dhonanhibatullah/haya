#include "application/netif/impl.h"

#include <stdlib.h>
#include <string.h>

#include "application/netif/impl_types.h"
#include "application/netif/impl_utils.h"
#include "domain/models/error.h"
#include "domain/models/network.h"
#include "domain/usecases/netif.h"

#define BASE_TAG "netif"

/* Helper Function Prototypes */

static dom_models_error_t get_ctx(
    dom_usecases_netif_t*  self,
    app_netif_impl_ctx_t** out
);

/* Contract Function Prototypes */

static dom_models_error_t get_all_impl(
    dom_usecases_netif_t* self,
    dom_models_network_t* out
);
static dom_models_error_t get_wifi_sta_impl(
    dom_usecases_netif_t*           self,
    dom_models_network_interface_t* out
);
static dom_models_error_t get_ethernet_impl(
    dom_usecases_netif_t*           self,
    dom_models_network_interface_t* out
);

/* Constructor and Destructor */

dom_usecases_netif_t* app_netif_impl_new(const app_netif_impl_cfg_t* cfg) {
    const char* tag = BASE_TAG "/new";

    dom_models_error_t err = app_netif_impl_validate_cfg(cfg);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return NULL;
    }

    app_netif_impl_ctx_t* ctx = (app_netif_impl_ctx_t*)calloc(1, sizeof(app_netif_impl_ctx_t));
    if (!ctx) {
        cfg->logger->error(cfg->logger, tag, "Failed to allocate Netif context: %s (%d)", dom_models_error_str(DOMAIN_MODELS_ERROR_MALLOC_FAILED), (int)DOMAIN_MODELS_ERROR_MALLOC_FAILED);
        return NULL;
    }

    memcpy(&ctx->cfg, cfg, sizeof(app_netif_impl_cfg_t));

    dom_usecases_netif_t* self = dom_usecases_netif_new(ctx);
    if (!self) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to allocate Netif usecase: %s (%d)", dom_models_error_str(DOMAIN_MODELS_ERROR_MALLOC_FAILED), (int)DOMAIN_MODELS_ERROR_MALLOC_FAILED);
        free(ctx);
        return NULL;
    }

    self->get_all      = get_all_impl;
    self->get_wifi_sta = get_wifi_sta_impl;
    self->get_ethernet = get_ethernet_impl;

    ctx->cfg.logger->info(ctx->cfg.logger, tag, "Netif created successfully");

    return self;
}

void app_netif_impl_delete(dom_usecases_netif_t* self) {
    const char* tag = BASE_TAG "/delete";

    if (!self) {
        return;
    }

    app_netif_impl_ctx_t* ctx = self->ctx;
    if (ctx) {
        ctx->cfg.logger->info(ctx->cfg.logger, tag, "Netif deleted successfully");
        free(ctx);
    }

    dom_usecases_netif_delete(self);
}

/* Contract Function Implementations */

static dom_models_error_t get_all_impl(
    dom_usecases_netif_t* self,
    dom_models_network_t* out
) {
    const char* tag = BASE_TAG "/get_all";

    app_netif_impl_ctx_t* ctx = NULL;
    dom_models_error_t    err = get_ctx(self, &ctx);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    if (!out) {
        err = DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Missing network output: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    err = ctx->cfg.network_interface->get_all(ctx->cfg.network_interface, out);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to get all network interfaces: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    ctx->cfg.logger->info(ctx->cfg.logger, tag, "Network interfaces retrieved successfully");

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t get_wifi_sta_impl(
    dom_usecases_netif_t*           self,
    dom_models_network_interface_t* out
) {
    const char* tag = BASE_TAG "/get_wifi_sta";

    app_netif_impl_ctx_t* ctx = NULL;
    dom_models_error_t    err = get_ctx(self, &ctx);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    if (!out) {
        err = DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Missing WiFi STA network interface output: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    err = ctx->cfg.network_interface->get_wifi_sta(ctx->cfg.network_interface, out);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to get WiFi STA network interface: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    ctx->cfg.logger->info(ctx->cfg.logger, tag, "WiFi STA network interface retrieved successfully");

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t get_ethernet_impl(
    dom_usecases_netif_t*           self,
    dom_models_network_interface_t* out
) {
    const char* tag = BASE_TAG "/get_ethernet";

    app_netif_impl_ctx_t* ctx = NULL;
    dom_models_error_t    err = get_ctx(self, &ctx);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    if (!out) {
        err = DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Missing Ethernet network interface output: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    err = ctx->cfg.network_interface->get_ethernet(ctx->cfg.network_interface, out);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to get Ethernet network interface: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    ctx->cfg.logger->info(ctx->cfg.logger, tag, "Ethernet network interface retrieved successfully");

    return DOMAIN_MODELS_ERROR_OK;
}

/* Helper Function Implementations */

static dom_models_error_t get_ctx(
    dom_usecases_netif_t*  self,
    app_netif_impl_ctx_t** out
) {
    if (!self || !self->ctx || !out) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    *out = self->ctx;

    return DOMAIN_MODELS_ERROR_OK;
}
