#ifndef APPLICATION_WIFIMAN_IMPL_UTILS_H
#define APPLICATION_WIFIMAN_IMPL_UTILS_H

#include <stddef.h>

#include "application/wifiman/impl_types.h"
#include "domain/models/error.h"
#include "domain/models/wifi.h"
#include "domain/usecases/wifiman.h"

#ifdef __cplusplus
extern "C" {
#endif

dom_models_error_t app_wifiman_impl_validate_cfg(const app_wifiman_impl_cfg_t* cfg);

dom_models_error_t app_wifiman_impl_validate_credential(const dom_models_wifi_sta_credential_t* credential);

size_t app_wifiman_impl_bounded_strlen(const char* value, size_t max_len);

void app_wifiman_impl_copy_cstr(char* dst, size_t dst_size, const char* src);

void app_wifiman_impl_credential_to_connect_config(
    dom_models_wifi_sta_connect_config_t* out,
    const dom_models_wifi_sta_credential_t* credential
);

dom_models_error_t app_wifiman_impl_load_ap_config(
    app_wifiman_impl_ctx_t* ctx,
    dom_models_wifi_ap_config_t* out
);

dom_models_error_t app_wifiman_impl_load_stored_credential(
    app_wifiman_impl_ctx_t* ctx,
    dom_models_wifi_sta_credential_t* out
);

dom_models_error_t app_wifiman_impl_load_stored_sta(
    app_wifiman_impl_ctx_t* ctx,
    dom_usecases_wifiman_stored_sta_t* out
);

#ifdef __cplusplus
}
#endif

#endif /* APPLICATION_WIFIMAN_IMPL_UTILS_H */
