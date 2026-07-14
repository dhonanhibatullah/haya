#ifndef APPLICATION_WIFIMAN_IMPL_H
#define APPLICATION_WIFIMAN_IMPL_H

#include "application/wifiman/impl_types.h"
#include "domain/usecases/wifiman.h"

#ifdef __cplusplus
extern "C" {
#endif

dom_usecases_wifiman_t* app_wifiman_impl_new(const app_wifiman_impl_cfg_t* cfg);

void app_wifiman_impl_delete(dom_usecases_wifiman_t* self);

#ifdef __cplusplus
}
#endif

#endif /* APPLICATION_WIFIMAN_IMPL_H */
