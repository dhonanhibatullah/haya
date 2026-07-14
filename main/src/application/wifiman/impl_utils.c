#include "application/wifiman/impl_utils.h"

#include <stdbool.h>
#include <string.h>

#include "application/wifiman/impl_types.h"
#include "domain/models/error.h"
#include "domain/models/wifi.h"

/* Helper Function Prototypes */

static bool has_wifi_functions(dom_contracts_device_wifi_t* wifi);
static bool has_wifi_repository_functions(dom_contracts_repository_wifi_t* repository);
static bool has_preloaded_repository_functions(dom_contracts_repository_preloaded_t* repository);
static bool has_network_interface_functions(dom_contracts_network_interface_t* network_interface);

dom_models_error_t app_wifiman_impl_validate_cfg(const app_wifiman_impl_cfg_t* cfg) {
    if (!cfg ||
        !cfg->logger ||
        !cfg->logger->error ||
        !cfg->logger->info ||
        !has_wifi_functions(cfg->wifi) ||
        !has_wifi_repository_functions(cfg->wifi_repository) ||
        !has_preloaded_repository_functions(cfg->preloaded_repository) ||
        !has_network_interface_functions(cfg->network_interface)) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    return DOMAIN_MODELS_ERROR_OK;
}

dom_models_error_t app_wifiman_impl_validate_credential(const dom_models_wifi_sta_credential_t* credential) {
    if (!credential) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    size_t ssid_len = app_wifiman_impl_bounded_strlen(credential->ssid, sizeof(credential->ssid));
    if (ssid_len == 0 || ssid_len >= sizeof(credential->ssid)) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    size_t password_len = app_wifiman_impl_bounded_strlen(credential->password, sizeof(credential->password));
    if (password_len >= sizeof(credential->password)) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    return DOMAIN_MODELS_ERROR_OK;
}

size_t app_wifiman_impl_bounded_strlen(const char* value, size_t max_len) {
    size_t len = 0;

    if (!value) {
        return 0;
    }

    while (len < max_len && value[len] != '\0') {
        len++;
    }

    return len;
}

void app_wifiman_impl_copy_cstr(char* dst, size_t dst_size, const char* src) {
    if (!dst || dst_size == 0) {
        return;
    }

    const char* value = src ? src : "";
    size_t      len   = app_wifiman_impl_bounded_strlen(value, dst_size - 1);
    if (len > 0) {
        memcpy(dst, value, len);
    }
    dst[len] = '\0';
}

void app_wifiman_impl_credential_to_connect_config(
    dom_models_wifi_sta_connect_config_t* out,
    const dom_models_wifi_sta_credential_t* credential
) {
    if (!out) {
        return;
    }

    memset(out, 0, sizeof(dom_models_wifi_sta_connect_config_t));
    if (!credential) {
        return;
    }

    app_wifiman_impl_copy_cstr(out->ssid, sizeof(out->ssid), credential->ssid);
    app_wifiman_impl_copy_cstr(out->password, sizeof(out->password), credential->password);
}

dom_models_error_t app_wifiman_impl_load_ap_config(
    app_wifiman_impl_ctx_t* ctx,
    dom_models_wifi_ap_config_t* out
) {
    if (!ctx || !out) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    memset(out, 0, sizeof(dom_models_wifi_ap_config_t));

    dom_models_error_t err = ctx->cfg.preloaded_repository->get_wifi_ap_ssid(
        ctx->cfg.preloaded_repository,
        out->ssid,
        sizeof(out->ssid)
    );
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    err = ctx->cfg.preloaded_repository->get_wifi_ap_pass(
        ctx->cfg.preloaded_repository,
        out->password,
        sizeof(out->password)
    );
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    bool password_empty = app_wifiman_impl_bounded_strlen(out->password, sizeof(out->password)) == 0;

    out->channel_set     = true;
    out->channel         = DOM_MODELS_WIFI_AP_DEFAULT_CHANNEL;
    out->auth_mode_set   = true;
    out->auth_mode       = password_empty ? DOM_MODELS_WIFI_AUTH_OPEN : DOM_MODELS_WIFI_AP_DEFAULT_AUTH_MODE;
    out->max_clients_set = true;
    out->max_clients     = DOM_MODELS_WIFI_AP_DEFAULT_MAX_CLIENTS;

    return DOMAIN_MODELS_ERROR_OK;
}

dom_models_error_t app_wifiman_impl_load_stored_credential(
    app_wifiman_impl_ctx_t* ctx,
    dom_models_wifi_sta_credential_t* out
) {
    if (!ctx || !out) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    return ctx->cfg.wifi_repository->get_sta_credential(ctx->cfg.wifi_repository, out);
}

dom_models_error_t app_wifiman_impl_load_stored_sta(
    app_wifiman_impl_ctx_t* ctx,
    dom_usecases_wifiman_stored_sta_t* out
) {
    if (!ctx || !out) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    memset(out, 0, sizeof(dom_usecases_wifiman_stored_sta_t));

    dom_models_wifi_sta_credential_t credential;
    dom_models_error_t               err = app_wifiman_impl_load_stored_credential(ctx, &credential);
    if (err == DOMAIN_MODELS_ERROR_NOT_FOUND) {
        return DOMAIN_MODELS_ERROR_OK;
    }
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    out->available = true;
    app_wifiman_impl_copy_cstr(out->ssid, sizeof(out->ssid), credential.ssid);

    return DOMAIN_MODELS_ERROR_OK;
}

/* Helper Function Implementations */

static bool has_wifi_functions(dom_contracts_device_wifi_t* wifi) {
    return wifi &&
           wifi->start &&
           wifi->stop &&
           wifi->set_mode &&
           wifi->get_status &&
           wifi->connect_sta &&
           wifi->disconnect_sta &&
           wifi->start_ap &&
           wifi->stop_ap &&
           wifi->start_scan &&
           wifi->get_scanned;
}

static bool has_wifi_repository_functions(dom_contracts_repository_wifi_t* repository) {
    return repository &&
           repository->get_sta_credential &&
           repository->set_sta_credential &&
           repository->clear_sta_credential;
}

static bool has_preloaded_repository_functions(dom_contracts_repository_preloaded_t* repository) {
    return repository &&
           repository->get_wifi_ap_ssid &&
           repository->get_wifi_ap_pass;
}

static bool has_network_interface_functions(dom_contracts_network_interface_t* network_interface) {
    return network_interface && network_interface->get_wifi_sta;
}
