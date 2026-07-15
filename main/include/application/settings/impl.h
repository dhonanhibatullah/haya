#ifndef APPLICATION_SETTINGS_IMPL_H
#define APPLICATION_SETTINGS_IMPL_H

#include "application/settings/impl_types.h"
#include "domain/usecases/settings.h"

#ifdef __cplusplus
extern "C" {
#endif

dom_usecases_settings_t* app_settings_impl_new(const app_settings_impl_cfg_t* cfg);

void app_settings_impl_delete(dom_usecases_settings_t* self);

#ifdef __cplusplus
}
#endif

#endif /* APPLICATION_SETTINGS_IMPL_H */
