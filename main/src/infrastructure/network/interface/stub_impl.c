#include "infrastructure/network/interface/stub_impl.h"

#include <stdlib.h>
#include <string.h>

#include "domain/contracts/network/interface.h"
#include "domain/models/network.h"
#include "infrastructure/network/interface/stub_impl_types.h"
#include "infrastructure/network/interface/stub_impl_utils.h"

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

dom_contracts_network_interface_t* inf_network_interface_stub_impl_new(const inf_network_interface_stub_impl_cfg_t* cfg) {
    inf_network_interface_stub_impl_ctx_t* ctx = (inf_network_interface_stub_impl_ctx_t*)calloc(1, sizeof(inf_network_interface_stub_impl_ctx_t));
    if (!ctx) {
        return NULL;
    }

    inf_network_interface_stub_impl_cfg_t default_cfg = INF_NETWORK_INTERFACE_STUB_IMPL_CFG_DEFAULT();
    dom_models_error_t                   err         = inf_network_interface_stub_impl_load_cfg(ctx, cfg ? cfg : &default_cfg);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        free(ctx);
        return NULL;
    }

    dom_contracts_network_interface_t* self = dom_contracts_network_interface_new(ctx);
    if (!self) {
        inf_network_interface_stub_impl_clear(ctx);
        free(ctx);
        return NULL;
    }

    self->get_all      = get_all_impl;
    self->get_wifi_sta = get_wifi_sta_impl;
    self->get_ethernet = get_ethernet_impl;

    return self;
}

void inf_network_interface_stub_impl_delete(dom_contracts_network_interface_t* self) {
    if (!self) {
        return;
    }

    inf_network_interface_stub_impl_ctx_t* ctx = self->ctx;
    if (ctx) {
        inf_network_interface_stub_impl_clear(ctx);
        free(ctx);
    }

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

    inf_network_interface_stub_impl_ctx_t* ctx = self->ctx;
    memcpy(out, &ctx->network, sizeof(dom_models_network_t));

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t get_wifi_sta_impl(
    dom_contracts_network_interface_t* self,
    dom_models_network_interface_t*    out
) {
    if (!self || !self->ctx || !out) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_network_interface_stub_impl_ctx_t* ctx = self->ctx;

    return inf_network_interface_stub_impl_find_wifi_sta(ctx, out);
}

static dom_models_error_t get_ethernet_impl(
    dom_contracts_network_interface_t* self,
    dom_models_network_interface_t*    out
) {
    if (!self || !self->ctx || !out) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    inf_network_interface_stub_impl_ctx_t* ctx = self->ctx;

    return inf_network_interface_stub_impl_find_ethernet(ctx, out);
}
