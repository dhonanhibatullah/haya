#ifndef APPLICATION_OTA_IMPL_TYPES_H
#define APPLICATION_OTA_IMPL_TYPES_H

#include <stdbool.h>

#include "domain/contracts/logger/leveled.h"
#include "domain/contracts/system/update.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    dom_contracts_logger_leveled_t* logger;
    dom_contracts_system_update_t* update;
} app_ota_impl_cfg_t;

typedef struct {
    app_ota_impl_cfg_t cfg;
    bool               updating;
} app_ota_impl_ctx_t;

#ifdef __cplusplus
}
#endif

#endif /* APPLICATION_OTA_IMPL_TYPES_H */
