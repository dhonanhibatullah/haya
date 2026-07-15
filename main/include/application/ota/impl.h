#ifndef APPLICATION_OTA_IMPL_H
#define APPLICATION_OTA_IMPL_H

#include "application/ota/impl_types.h"
#include "domain/usecases/ota.h"

#ifdef __cplusplus
extern "C" {
#endif

dom_usecases_ota_t* app_ota_impl_new(const app_ota_impl_cfg_t* cfg);

void app_ota_impl_delete(dom_usecases_ota_t* self);

#ifdef __cplusplus
}
#endif

#endif /* APPLICATION_OTA_IMPL_H */
