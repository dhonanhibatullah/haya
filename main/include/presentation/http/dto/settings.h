#ifndef PRESENTATION_HTTP_DTO_SETTINGS_H
#define PRESENTATION_HTTP_DTO_SETTINGS_H

#include <stdbool.h>

#include "cJSON.h"
#include "domain/models/error.h"
#include "domain/usecases/settings.h"

#ifdef __cplusplus
extern "C" {
#endif

dom_models_error_t pres_http_dto_settings_parse_preloaded_update(
    cJSON*                                    json,
    dom_usecases_settings_preloaded_update_t* out
);

cJSON* pres_http_dto_settings_snapshot_to_json(const dom_usecases_settings_snapshot_t* snapshot);

cJSON* pres_http_dto_settings_preloaded_updated_to_json(bool restart_required);

cJSON* pres_http_dto_settings_restart_required_to_json(bool restart_required);

cJSON* pres_http_dto_settings_accepted_to_json(void);

#ifdef __cplusplus
}
#endif

#endif /* PRESENTATION_HTTP_DTO_SETTINGS_H */
