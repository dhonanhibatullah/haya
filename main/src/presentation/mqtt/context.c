#include "presentation/mqtt/context.h"

#include <stdlib.h>

pres_mqtt_context_t* pres_mqtt_context_new(
    dom_contracts_logger_leveled_t*       logger,
    dom_contracts_repository_preloaded_t* preloaded_repository,
    dom_usecases_settings_t*              settings,
    dom_usecases_ota_t*                   ota
) {
    if (!logger || !preloaded_repository || !settings || !ota) {
        return NULL;
    }

    pres_mqtt_context_t* self = (pres_mqtt_context_t*)calloc(1, sizeof(pres_mqtt_context_t));
    if (!self) {
        return NULL;
    }

    self->logger               = logger;
    self->preloaded_repository = preloaded_repository;
    self->settings             = settings;
    self->ota                  = ota;

    dom_models_error_t err = preloaded_repository->get_device_id_str(
        preloaded_repository,
        self->device_id_str,
        sizeof(self->device_id_str)
    );
    if (err != DOMAIN_MODELS_ERROR_OK) {
        free(self);
        return NULL;
    }

    return self;
}

void pres_mqtt_context_delete(pres_mqtt_context_t* self) {
    if (!self) {
        return;
    }
    free(self);
}
