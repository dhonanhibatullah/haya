#include "infrastructure/network/interface/stub_impl_utils.h"

#include <string.h>

/* Helper Function Prototypes */

static void normalize_network(dom_models_network_t* network);

dom_models_error_t inf_network_interface_stub_impl_copy_cstr(char* out, size_t out_size, const char* value) {
    if (!out || out_size == 0) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    const char* src = value ? value : "";
    size_t      len = strlen(src);
    if (len >= out_size) {
        len = out_size - 1;
    }

    memcpy(out, src, len);
    out[len] = '\0';

    return DOMAIN_MODELS_ERROR_OK;
}

dom_models_error_t inf_network_interface_stub_impl_load_cfg(
    inf_network_interface_stub_impl_ctx_t* ctx,
    const inf_network_interface_stub_impl_cfg_t* cfg
) {
    if (!ctx || !cfg) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_network_interface_stub_impl_clear(ctx);

    dom_models_error_t err = inf_network_interface_stub_impl_copy_cstr(ctx->sta_if_key, sizeof(ctx->sta_if_key), cfg->sta_if_key);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        inf_network_interface_stub_impl_clear(ctx);
        return err;
    }

    err = inf_network_interface_stub_impl_copy_cstr(ctx->eth_if_key, sizeof(ctx->eth_if_key), cfg->eth_if_key);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        inf_network_interface_stub_impl_clear(ctx);
        return err;
    }

    memcpy(&ctx->network, &cfg->network, sizeof(dom_models_network_t));
    normalize_network(&ctx->network);

    return DOMAIN_MODELS_ERROR_OK;
}

dom_models_error_t inf_network_interface_stub_impl_find_wifi_sta(
    const inf_network_interface_stub_impl_ctx_t* ctx,
    dom_models_network_interface_t*              out
) {
    if (!ctx || !out) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    memset(out, 0, sizeof(dom_models_network_interface_t));

    for (size_t i = 0; i < ctx->network.count; i++) {
        const dom_models_network_interface_t* current = &ctx->network.interfaces[i];
        if (strncmp(current->if_key, ctx->sta_if_key, sizeof(current->if_key)) == 0) {
            memcpy(out, current, sizeof(dom_models_network_interface_t));
            return DOMAIN_MODELS_ERROR_OK;
        }
    }

    for (size_t i = 0; i < ctx->network.count; i++) {
        const dom_models_network_interface_t* current = &ctx->network.interfaces[i];
        if (current->type == DOM_MODELS_NETWORK_INTERFACE_TYPE_WIFI_STA) {
            memcpy(out, current, sizeof(dom_models_network_interface_t));
            return DOMAIN_MODELS_ERROR_OK;
        }
    }

    return DOMAIN_MODELS_ERROR_NOT_FOUND;
}

dom_models_error_t inf_network_interface_stub_impl_find_ethernet(
    const inf_network_interface_stub_impl_ctx_t* ctx,
    dom_models_network_interface_t*              out
) {
    if (!ctx || !out) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    memset(out, 0, sizeof(dom_models_network_interface_t));

    for (size_t i = 0; i < ctx->network.count; i++) {
        const dom_models_network_interface_t* current = &ctx->network.interfaces[i];
        if (strncmp(current->if_key, ctx->eth_if_key, sizeof(current->if_key)) == 0) {
            memcpy(out, current, sizeof(dom_models_network_interface_t));
            return DOMAIN_MODELS_ERROR_OK;
        }
    }

    for (size_t i = 0; i < ctx->network.count; i++) {
        const dom_models_network_interface_t* current = &ctx->network.interfaces[i];
        if (current->type == DOM_MODELS_NETWORK_INTERFACE_TYPE_ETHERNET) {
            memcpy(out, current, sizeof(dom_models_network_interface_t));
            return DOMAIN_MODELS_ERROR_OK;
        }
    }

    return DOMAIN_MODELS_ERROR_NOT_FOUND;
}

void inf_network_interface_stub_impl_clear(inf_network_interface_stub_impl_ctx_t* ctx) {
    if (!ctx) {
        return;
    }

    memset(ctx, 0, sizeof(inf_network_interface_stub_impl_ctx_t));
}

/* Helper Function Implementations */

static void normalize_network(dom_models_network_t* network) {
    if (!network) {
        return;
    }

    if (network->count > DOM_MODELS_NETWORK_MAX_INTERFACES) {
        network->count     = DOM_MODELS_NETWORK_MAX_INTERFACES;
        network->truncated = true;
    }
    if (network->total_count < network->count) {
        network->total_count = network->count;
    }
    if (network->count < network->total_count) {
        network->truncated = true;
    }
}
