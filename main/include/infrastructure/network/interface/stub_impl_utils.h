#ifndef INFRASTRUCTURE_NETWORK_INTERFACE_STUB_IMPL_UTILS_H
#define INFRASTRUCTURE_NETWORK_INTERFACE_STUB_IMPL_UTILS_H

#include <stddef.h>

#include "domain/models/error.h"
#include "domain/models/network.h"
#include "infrastructure/network/interface/stub_impl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

dom_models_error_t inf_network_interface_stub_impl_copy_cstr(char* out, size_t out_size, const char* value);

dom_models_error_t inf_network_interface_stub_impl_load_cfg(
    inf_network_interface_stub_impl_ctx_t* ctx,
    const inf_network_interface_stub_impl_cfg_t* cfg
);

dom_models_error_t inf_network_interface_stub_impl_find_wifi_sta(
    const inf_network_interface_stub_impl_ctx_t* ctx,
    dom_models_network_interface_t* out
);

void inf_network_interface_stub_impl_clear(inf_network_interface_stub_impl_ctx_t* ctx);

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_NETWORK_INTERFACE_STUB_IMPL_UTILS_H */
