#include "application/ota/impl_utils.h"

#include <stdbool.h>
#include <string.h>

#include "application/ota/impl_types.h"
#include "domain/contracts/logger/leveled.h"
#include "domain/contracts/system/update.h"
#include "domain/models/error.h"
#include "domain/models/update.h"

static bool has_logger_functions(dom_contracts_logger_leveled_t* logger);
static bool has_system_update_functions(dom_contracts_system_update_t* update);
static bool has_system_restart_functions(dom_contracts_system_restart_t* restart);

dom_models_error_t app_ota_impl_validate_cfg(const app_ota_impl_cfg_t* cfg) {
    if (!cfg ||
        !has_logger_functions(cfg->logger) ||
        !has_system_update_functions(cfg->update) ||
        !has_system_restart_functions(cfg->restart)) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    return DOMAIN_MODELS_ERROR_OK;
}

dom_models_error_t app_ota_impl_validate_update_info(const dom_models_update_info_t* update_info) {
    if (!update_info) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    if (strlen(update_info->firmware_url) == 0) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    if (update_info->firmware_size == 0) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    if (strlen(update_info->firmware_checksum) == 0) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    return DOMAIN_MODELS_ERROR_OK;
}

/* Helper Function Implementations */

static bool has_logger_functions(dom_contracts_logger_leveled_t* logger) {
    return logger &&
           logger->error &&
           logger->warn &&
           logger->info &&
           logger->debug;
}

static bool has_system_update_functions(dom_contracts_system_update_t* update) {
    return update &&
           update->update &&
           update->validate &&
           update->rollback;
}

static bool has_system_restart_functions(dom_contracts_system_restart_t* restart) {
    return restart &&
           restart->restart;
}
