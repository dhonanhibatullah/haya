#ifndef APPLICATION_SETTINGS_IMPL_UTILS_H
#define APPLICATION_SETTINGS_IMPL_UTILS_H

#include <stdbool.h>

#include "application/settings/impl_types.h"
#include "domain/models/error.h"
#include "domain/usecases/settings.h"

#ifdef __cplusplus
extern "C" {
#endif

dom_models_error_t app_settings_impl_validate_cfg(const app_settings_impl_cfg_t* cfg);

dom_models_error_t app_settings_impl_load_snapshot(
    app_settings_impl_ctx_t*          ctx,
    dom_usecases_settings_snapshot_t* out
);

bool app_settings_impl_has_preloaded_update(const dom_usecases_settings_preloaded_update_t* update);

#ifdef __cplusplus
}
#endif

#endif /* APPLICATION_SETTINGS_IMPL_UTILS_H */
