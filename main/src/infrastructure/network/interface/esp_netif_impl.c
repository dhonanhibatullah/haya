#include "infrastructure/network/interface/esp_netif_impl.h"

#include <stdlib.h>
#include <string.h>

#include "domain/contracts/network/interface.h"
#include "domain/models/network.h"
#include "esp_netif.h"
#include "infrastructure/network/interface/esp_netif_impl_types.h"
#include "infrastructure/network/interface/esp_netif_impl_utils.h"

/* Contract Function Prototypes */

static dom_models_error_t get_all_impl(
    dom_contracts_network_interface_t* self,
    dom_models_network_t*              out
);
static dom_models_error_t get_wifi_sta_impl(
    dom_contracts_network_interface_t* self,
    dom_models_network_interface_t*    out
);
static dom_models_error_t get_ethernet_impl(
    dom_contracts_network_interface_t* self,
    dom_models_network_interface_t*    out
);

/* Constructor and Destructor */

dom_contracts_network_interface_t* inf_network_interface_esp_netif_impl_new(const inf_network_interface_esp_netif_impl_cfg_t* cfg) {
    inf_network_interface_esp_netif_impl_ctx_t* ctx = (inf_network_interface_esp_netif_impl_ctx_t*)calloc(1, sizeof(inf_network_interface_esp_netif_impl_ctx_t));
    if (!ctx) {
        return NULL;
    }

    inf_network_interface_esp_netif_impl_cfg_t default_cfg = INF_NETWORK_INTERFACE_ESP_NETIF_IMPL_CFG_DEFAULT();
    memcpy(&ctx->cfg, cfg ? cfg : &default_cfg, sizeof(inf_network_interface_esp_netif_impl_cfg_t));
    if (!ctx->cfg.sta_if_key) {
        ctx->cfg.sta_if_key = default_cfg.sta_if_key;
    }
    if (!ctx->cfg.eth_if_key) {
        ctx->cfg.eth_if_key = default_cfg.eth_if_key;
    }

    dom_contracts_network_interface_t* self = dom_contracts_network_interface_new(ctx);
    if (!self) {
        free(ctx);
        return NULL;
    }

    self->get_all      = get_all_impl;
    self->get_wifi_sta = get_wifi_sta_impl;
    self->get_ethernet = get_ethernet_impl;

    return self;
}

void inf_network_interface_esp_netif_impl_delete(dom_contracts_network_interface_t* self) {
    if (!self) {
        return;
    }

    free(self->ctx);
    dom_contracts_network_interface_delete(self);
}

/* Contract Function Implementations */

static dom_models_error_t get_all_impl(
    dom_contracts_network_interface_t* self,
    dom_models_network_t*              out
) {
    if (!self || !self->ctx || !out) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    memset(out, 0, sizeof(dom_models_network_t));
    out->total_count = esp_netif_get_nr_of_ifs();

    esp_netif_t* netif = NULL;
    while ((netif = esp_netif_next_unsafe(netif)) != NULL) {
        if (out->count < DOM_MODELS_NETWORK_MAX_INTERFACES) {
            inf_network_interface_esp_netif_impl_fill_interface(netif, &out->interfaces[out->count]);
            out->count++;
        } else {
            out->truncated = true;
        }
    }

    if (out->count < out->total_count) {
        out->truncated = true;
    }

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t get_wifi_sta_impl(
    dom_contracts_network_interface_t* self,
    dom_models_network_interface_t*    out
) {
    if (!self || !self->ctx || !out) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_network_interface_esp_netif_impl_ctx_t* ctx = self->ctx;

    memset(out, 0, sizeof(dom_models_network_interface_t));

    esp_netif_t* netif = esp_netif_get_handle_from_ifkey(ctx->cfg.sta_if_key);
    if (!netif) {
        return DOMAIN_MODELS_ERROR_NOT_FOUND;
    }

    inf_network_interface_esp_netif_impl_fill_interface(netif, out);

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t get_ethernet_impl(
    dom_contracts_network_interface_t* self,
    dom_models_network_interface_t*    out
) {
    if (!self || !self->ctx || !out) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_network_interface_esp_netif_impl_ctx_t* ctx = self->ctx;

    memset(out, 0, sizeof(dom_models_network_interface_t));

    esp_netif_t* netif = esp_netif_get_handle_from_ifkey(ctx->cfg.eth_if_key);
    if (!netif) {
        return DOMAIN_MODELS_ERROR_NOT_FOUND;
    }

    inf_network_interface_esp_netif_impl_fill_interface(netif, out);

    return DOMAIN_MODELS_ERROR_OK;
}
