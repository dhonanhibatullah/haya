#include "application/wifiman/impl.h"

#include <stdlib.h>
#include <string.h>

#include "application/wifiman/impl_types.h"
#include "application/wifiman/impl_utils.h"
#include "domain/models/error.h"
#include "domain/models/wifi.h"
#include "domain/usecases/wifiman.h"

#define BASE_TAG "wifiman"

/* Helper Function Prototypes */

static dom_models_error_t ensure_apsta(
    app_wifiman_impl_ctx_t* ctx,
    const char*             tag
);

static dom_models_error_t get_ctx(
    dom_usecases_wifiman_t* self,
    app_wifiman_impl_ctx_t** out
);

/* Contract Function Prototypes */

static dom_models_error_t start_impl(
    dom_usecases_wifiman_t* self
);
static dom_models_error_t stop_impl(
    dom_usecases_wifiman_t* self
);
static dom_models_error_t start_scan_impl(
    dom_usecases_wifiman_t*              self,
    const dom_models_wifi_scan_config_t* config
);
static dom_models_error_t get_scan_result_impl(
    dom_usecases_wifiman_t*        self,
    dom_models_wifi_scan_result_t* out
);
static dom_models_error_t get_status_impl(
    dom_usecases_wifiman_t*        self,
    dom_usecases_wifiman_status_t* out
);
static dom_models_error_t connect_sta_impl(
    dom_usecases_wifiman_t*                  self,
    const dom_models_wifi_sta_credential_t* credential
);
static dom_models_error_t connect_stored_sta_impl(
    dom_usecases_wifiman_t* self
);
static dom_models_error_t disconnect_sta_impl(
    dom_usecases_wifiman_t* self
);
static dom_models_error_t get_stored_sta_impl(
    dom_usecases_wifiman_t*            self,
    dom_usecases_wifiman_stored_sta_t* out
);
static dom_models_error_t set_sta_credential_impl(
    dom_usecases_wifiman_t*                  self,
    const dom_models_wifi_sta_credential_t* credential
);
static dom_models_error_t forget_sta_credential_impl(
    dom_usecases_wifiman_t* self
);
static dom_models_error_t need_reconnect_impl(
    dom_usecases_wifiman_t* self,
    bool*                   out
);
static dom_models_error_t try_reconnect_impl(
    dom_usecases_wifiman_t* self,
    bool*                   attempted
);

/* Constructor and Destructor */

dom_usecases_wifiman_t* app_wifiman_impl_new(const app_wifiman_impl_cfg_t* cfg) {
    const char* tag = BASE_TAG"/new";

    dom_models_error_t err = app_wifiman_impl_validate_cfg(cfg);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return NULL;
    }

    app_wifiman_impl_ctx_t* ctx = (app_wifiman_impl_ctx_t*)calloc(1, sizeof(app_wifiman_impl_ctx_t));
    if (!ctx) {
        cfg->logger->error(cfg->logger, tag, "Failed to allocate WiFiMan context: %s (%d)", dom_models_error_str(DOMAIN_MODELS_ERROR_MALLOC_FAILED), (int)DOMAIN_MODELS_ERROR_MALLOC_FAILED);
        return NULL;
    }

    memcpy(&ctx->cfg, cfg, sizeof(app_wifiman_impl_cfg_t));
    if (ctx->cfg.reconnect_max_trials == 0) {
        ctx->cfg.reconnect_max_trials = APP_WIFIMAN_IMPL_DEFAULT_RECONNECT_MAX_TRIALS;
    }

    dom_usecases_wifiman_t* self = dom_usecases_wifiman_new(ctx);
    if (!self) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to allocate WiFiMan usecase: %s (%d)", dom_models_error_str(DOMAIN_MODELS_ERROR_MALLOC_FAILED), (int)DOMAIN_MODELS_ERROR_MALLOC_FAILED);
        free(ctx);
        return NULL;
    }

    self->start                 = start_impl;
    self->stop                  = stop_impl;
    self->start_scan            = start_scan_impl;
    self->get_scan_result       = get_scan_result_impl;
    self->get_status            = get_status_impl;
    self->connect_sta           = connect_sta_impl;
    self->connect_stored_sta    = connect_stored_sta_impl;
    self->disconnect_sta        = disconnect_sta_impl;
    self->get_stored_sta        = get_stored_sta_impl;
    self->set_sta_credential    = set_sta_credential_impl;
    self->forget_sta_credential = forget_sta_credential_impl;
    self->need_reconnect        = need_reconnect_impl;
    self->try_reconnect         = try_reconnect_impl;

    ctx->cfg.logger->info(ctx->cfg.logger, tag, "WiFiMan created successfully");

    return self;
}

void app_wifiman_impl_delete(dom_usecases_wifiman_t* self) {
    const char* tag = BASE_TAG"/delete";

    if (!self) {
        return;
    }

    app_wifiman_impl_ctx_t* ctx = self->ctx;
    if (ctx) {
        ctx->cfg.logger->info(ctx->cfg.logger, tag, "WiFiMan deleted successfully");
        free(ctx);
    }

    dom_usecases_wifiman_delete(self);
}

/* Contract Function Implementations */

static dom_models_error_t start_impl(
    dom_usecases_wifiman_t* self
) {
    const char* tag = BASE_TAG"/start";

    app_wifiman_impl_ctx_t* ctx = NULL;
    dom_models_error_t      err = get_ctx(self, &ctx);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    err = ensure_apsta(ctx, tag);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    ctx->cfg.logger->info(ctx->cfg.logger, tag, "WiFiMan started successfully");

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t stop_impl(
    dom_usecases_wifiman_t* self
) {
    const char* tag = BASE_TAG"/stop";

    app_wifiman_impl_ctx_t* ctx = NULL;
    dom_models_error_t      err = get_ctx(self, &ctx);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    if (!ctx->started) {
        ctx->auto_reconnect_enabled = false;
        ctx->reconnect_trial_count  = 0;
        ctx->cfg.logger->info(ctx->cfg.logger, tag, "WiFiMan stopped successfully");
        return DOMAIN_MODELS_ERROR_OK;
    }

    dom_models_wifi_status_t status;
    err = ctx->cfg.wifi->get_status(ctx->cfg.wifi, &status);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to get WiFi status before stop: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    if (status.connected) {
        err = ctx->cfg.wifi->disconnect_sta(ctx->cfg.wifi);
        if (err != DOMAIN_MODELS_ERROR_OK) {
            ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to disconnect STA: %s (%d)", dom_models_error_str(err), (int)err);
            return err;
        }
    }

    err = ctx->cfg.wifi->stop_ap(ctx->cfg.wifi);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to stop AP: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    err = ctx->cfg.wifi->stop(ctx->cfg.wifi);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to stop WiFi: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    ctx->started                = false;
    ctx->auto_reconnect_enabled = false;
    ctx->reconnect_trial_count  = 0;

    ctx->cfg.logger->info(ctx->cfg.logger, tag, "WiFiMan stopped successfully");

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t start_scan_impl(
    dom_usecases_wifiman_t*              self,
    const dom_models_wifi_scan_config_t* config
) {
    const char* tag = BASE_TAG"/start_scan";

    app_wifiman_impl_ctx_t* ctx = NULL;
    dom_models_error_t      err = get_ctx(self, &ctx);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    err = ensure_apsta(ctx, tag);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    err = ctx->cfg.wifi->start_scan(ctx->cfg.wifi, config);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to start WiFi scan: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    ctx->cfg.logger->info(ctx->cfg.logger, tag, "WiFi scan started successfully");

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t get_scan_result_impl(
    dom_usecases_wifiman_t*        self,
    dom_models_wifi_scan_result_t* out
) {
    const char* tag = BASE_TAG"/get_scan_result";

    app_wifiman_impl_ctx_t* ctx = NULL;
    dom_models_error_t      err = get_ctx(self, &ctx);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }
    if (!out) {
        err = DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Missing scan result output: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    err = ctx->cfg.wifi->get_scanned(ctx->cfg.wifi, out);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to get WiFi scan result: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    ctx->cfg.logger->info(ctx->cfg.logger, tag, "WiFi scan result loaded successfully");

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t get_status_impl(
    dom_usecases_wifiman_t*        self,
    dom_usecases_wifiman_status_t* out
) {
    const char* tag = BASE_TAG"/get_status";

    app_wifiman_impl_ctx_t* ctx = NULL;
    dom_models_error_t      err = get_ctx(self, &ctx);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }
    if (!out) {
        err = DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Missing status output: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    memset(out, 0, sizeof(dom_usecases_wifiman_status_t));

    err = ctx->cfg.wifi->get_status(ctx->cfg.wifi, &out->wifi);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to get WiFi status: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    if (out->wifi.connected) {
        ctx->reconnect_trial_count = 0;
    }

    err = app_wifiman_impl_load_stored_sta(ctx, &out->stored_sta);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to load stored STA credential view: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    err = ctx->cfg.network_interface->get_wifi_sta(ctx->cfg.network_interface, &out->sta_netif);
    if (err == DOMAIN_MODELS_ERROR_OK) {
        out->sta_netif_available = true;
    } else if (err == DOMAIN_MODELS_ERROR_NOT_FOUND) {
        memset(&out->sta_netif, 0, sizeof(dom_models_network_interface_t));
        out->sta_netif_available = false;
    } else {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to load STA network interface status: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    out->auto_reconnect_enabled = ctx->auto_reconnect_enabled;
    out->reconnect_trial_count  = ctx->reconnect_trial_count;
    out->reconnect_max_trials   = ctx->cfg.reconnect_max_trials;

    ctx->cfg.logger->info(ctx->cfg.logger, tag, "WiFiMan status loaded successfully");

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t connect_sta_impl(
    dom_usecases_wifiman_t*                  self,
    const dom_models_wifi_sta_credential_t* credential
) {
    const char* tag = BASE_TAG"/connect_sta";

    app_wifiman_impl_ctx_t* ctx = NULL;
    dom_models_error_t      err = get_ctx(self, &ctx);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    err = app_wifiman_impl_validate_credential(credential);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Invalid STA credential: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    err = ensure_apsta(ctx, tag);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    dom_models_wifi_sta_connect_config_t config;
    app_wifiman_impl_credential_to_connect_config(&config, credential);

    err = ctx->cfg.wifi->connect_sta(ctx->cfg.wifi, &config);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to connect STA: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    err = ctx->cfg.wifi_repository->set_sta_credential(ctx->cfg.wifi_repository, credential);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to store STA credential: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    ctx->auto_reconnect_enabled = true;
    ctx->reconnect_trial_count  = 0;

    ctx->cfg.logger->info(ctx->cfg.logger, tag, "STA connection request accepted successfully");

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t connect_stored_sta_impl(
    dom_usecases_wifiman_t* self
) {
    const char* tag = BASE_TAG"/connect_stored_sta";

    app_wifiman_impl_ctx_t* ctx = NULL;
    dom_models_error_t      err = get_ctx(self, &ctx);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    dom_models_wifi_sta_credential_t credential;
    err = app_wifiman_impl_load_stored_credential(ctx, &credential);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to load stored STA credential: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    err = app_wifiman_impl_validate_credential(&credential);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Stored STA credential is invalid: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    err = ensure_apsta(ctx, tag);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    dom_models_wifi_sta_connect_config_t config;
    app_wifiman_impl_credential_to_connect_config(&config, &credential);

    err = ctx->cfg.wifi->connect_sta(ctx->cfg.wifi, &config);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to connect stored STA: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    ctx->auto_reconnect_enabled = true;
    ctx->reconnect_trial_count  = 0;

    ctx->cfg.logger->info(ctx->cfg.logger, tag, "Stored STA connection request accepted successfully");

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t disconnect_sta_impl(
    dom_usecases_wifiman_t* self
) {
    const char* tag = BASE_TAG"/disconnect_sta";

    app_wifiman_impl_ctx_t* ctx = NULL;
    dom_models_error_t      err = get_ctx(self, &ctx);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    err = ctx->cfg.wifi->disconnect_sta(ctx->cfg.wifi);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        dom_models_wifi_status_t status;
        dom_models_error_t       status_err = ctx->cfg.wifi->get_status(ctx->cfg.wifi, &status);
        if (status_err == DOMAIN_MODELS_ERROR_OK && !status.connected) {
            ctx->auto_reconnect_enabled = false;
            ctx->reconnect_trial_count  = 0;
            ctx->cfg.logger->info(ctx->cfg.logger, tag, "STA disconnected successfully");
            return DOMAIN_MODELS_ERROR_OK;
        }

        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to disconnect STA: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    ctx->auto_reconnect_enabled = false;
    ctx->reconnect_trial_count  = 0;

    ctx->cfg.logger->info(ctx->cfg.logger, tag, "STA disconnected successfully");

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t get_stored_sta_impl(
    dom_usecases_wifiman_t*            self,
    dom_usecases_wifiman_stored_sta_t* out
) {
    const char* tag = BASE_TAG"/get_stored_sta";

    app_wifiman_impl_ctx_t* ctx = NULL;
    dom_models_error_t      err = get_ctx(self, &ctx);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }
    if (!out) {
        err = DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Missing stored STA output: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    err = app_wifiman_impl_load_stored_sta(ctx, out);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to load stored STA credential view: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    ctx->cfg.logger->info(ctx->cfg.logger, tag, "Stored STA credential view loaded successfully");

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t set_sta_credential_impl(
    dom_usecases_wifiman_t*                  self,
    const dom_models_wifi_sta_credential_t* credential
) {
    const char* tag = BASE_TAG"/set_sta_credential";

    app_wifiman_impl_ctx_t* ctx = NULL;
    dom_models_error_t      err = get_ctx(self, &ctx);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    err = app_wifiman_impl_validate_credential(credential);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Invalid STA credential: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    err = ctx->cfg.wifi_repository->set_sta_credential(ctx->cfg.wifi_repository, credential);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to store STA credential: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    ctx->auto_reconnect_enabled = true;
    ctx->reconnect_trial_count  = 0;

    ctx->cfg.logger->info(ctx->cfg.logger, tag, "STA credential stored successfully");

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t forget_sta_credential_impl(
    dom_usecases_wifiman_t* self
) {
    const char* tag = BASE_TAG"/forget_sta_credential";

    app_wifiman_impl_ctx_t* ctx = NULL;
    dom_models_error_t      err = get_ctx(self, &ctx);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    err = ctx->cfg.wifi_repository->clear_sta_credential(ctx->cfg.wifi_repository);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to forget STA credential: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    ctx->auto_reconnect_enabled = false;
    ctx->reconnect_trial_count  = 0;

    ctx->cfg.logger->info(ctx->cfg.logger, tag, "STA credential forgotten successfully");

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t need_reconnect_impl(
    dom_usecases_wifiman_t* self,
    bool*                   out
) {
    const char* tag = BASE_TAG"/need_reconnect";

    app_wifiman_impl_ctx_t* ctx = NULL;
    dom_models_error_t      err = get_ctx(self, &ctx);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }
    if (!out) {
        err = DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Missing reconnect output: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    *out = false;

    if (!ctx->auto_reconnect_enabled) {
        ctx->cfg.logger->info(ctx->cfg.logger, tag, "Reconnect is not needed because auto reconnect is disabled");
        return DOMAIN_MODELS_ERROR_OK;
    }

    dom_models_wifi_status_t status;
    err = ctx->cfg.wifi->get_status(ctx->cfg.wifi, &status);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to get WiFi status for reconnect decision: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    if (status.connected) {
        ctx->reconnect_trial_count = 0;
        ctx->cfg.logger->info(ctx->cfg.logger, tag, "Reconnect is not needed because STA is connected");
        return DOMAIN_MODELS_ERROR_OK;
    }

    if (ctx->reconnect_trial_count >= ctx->cfg.reconnect_max_trials) {
        ctx->cfg.logger->info(ctx->cfg.logger, tag, "Reconnect is not needed because retry limit is reached");
        return DOMAIN_MODELS_ERROR_OK;
    }

    dom_models_wifi_sta_credential_t credential;
    err = app_wifiman_impl_load_stored_credential(ctx, &credential);
    if (err == DOMAIN_MODELS_ERROR_NOT_FOUND) {
        ctx->cfg.logger->info(ctx->cfg.logger, tag, "Reconnect is not needed because no stored credential is available");
        return DOMAIN_MODELS_ERROR_OK;
    }
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to load stored credential for reconnect decision: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    err = app_wifiman_impl_validate_credential(&credential);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Stored credential is invalid for reconnect: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    *out = true;

    ctx->cfg.logger->info(ctx->cfg.logger, tag, "Reconnect is needed");

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t try_reconnect_impl(
    dom_usecases_wifiman_t* self,
    bool*                   attempted
) {
    const char* tag = BASE_TAG"/try_reconnect";

    app_wifiman_impl_ctx_t* ctx = NULL;
    dom_models_error_t      err = get_ctx(self, &ctx);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }
    if (!attempted) {
        err = DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Missing reconnect attempted output: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    *attempted = false;

    bool needed = false;
    err = need_reconnect_impl(self, &needed);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to evaluate reconnect need: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    if (!needed) {
        ctx->cfg.logger->info(ctx->cfg.logger, tag, "Reconnect was not attempted because it is not needed");
        return DOMAIN_MODELS_ERROR_OK;
    }

    dom_models_wifi_sta_credential_t credential;
    err = app_wifiman_impl_load_stored_credential(ctx, &credential);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to load stored credential for reconnect: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    err = ensure_apsta(ctx, tag);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    dom_models_wifi_sta_connect_config_t config;
    app_wifiman_impl_credential_to_connect_config(&config, &credential);

    ctx->reconnect_trial_count++;
    *attempted = true;

    err = ctx->cfg.wifi->connect_sta(ctx->cfg.wifi, &config);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to reconnect using stored STA credential: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    ctx->cfg.logger->info(ctx->cfg.logger, tag, "STA reconnect request accepted successfully");

    return DOMAIN_MODELS_ERROR_OK;
}

/* Helper Function Implementations */

static dom_models_error_t ensure_apsta(
    app_wifiman_impl_ctx_t* ctx,
    const char*             tag
) {
    if (!ctx) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    if (ctx->started) {
        return DOMAIN_MODELS_ERROR_OK;
    }

    dom_models_error_t err = ctx->cfg.wifi->set_mode(ctx->cfg.wifi, DOM_MODELS_WIFI_MODE_APSTA);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to set APSTA mode: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    err = ctx->cfg.wifi->start(ctx->cfg.wifi);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to start WiFi: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    dom_models_wifi_ap_config_t ap_config;
    err = app_wifiman_impl_load_ap_config(ctx, &ap_config);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to load AP configuration: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    err = ctx->cfg.wifi->start_ap(ctx->cfg.wifi, &ap_config);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to start AP: %s (%d)", dom_models_error_str(err), (int)err);
        return err;
    }

    ctx->started = true;

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t get_ctx(
    dom_usecases_wifiman_t* self,
    app_wifiman_impl_ctx_t** out
) {
    if (!self || !self->ctx || !out) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    *out = self->ctx;

    return DOMAIN_MODELS_ERROR_OK;
}
