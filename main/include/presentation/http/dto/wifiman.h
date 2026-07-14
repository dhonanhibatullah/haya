#ifndef PRESENTATION_HTTP_DTO_WIFIMAN_H
#define PRESENTATION_HTTP_DTO_WIFIMAN_H

#include <stdbool.h>

#include "cJSON.h"
#include "domain/models/error.h"
#include "domain/models/wifi.h"
#include "domain/usecases/wifiman.h"

#ifdef __cplusplus
extern "C" {
#endif

dom_models_error_t pres_http_dto_wifiman_parse_sta_credential(
    cJSON*                              json,
    dom_models_wifi_sta_credential_t*   out
);

dom_models_error_t pres_http_dto_wifiman_parse_scan_config(
    cJSON*                           json,
    dom_models_wifi_scan_config_t*   out
);

cJSON* pres_http_dto_wifiman_status_to_json(const dom_usecases_wifiman_status_t* status);

cJSON* pres_http_dto_wifiman_scan_result_to_json(const dom_models_wifi_scan_result_t* result);

cJSON* pres_http_dto_wifiman_stored_sta_to_json(const dom_usecases_wifiman_stored_sta_t* stored_sta);

cJSON* pres_http_dto_wifiman_reconnect_need_to_json(bool needed);

cJSON* pres_http_dto_wifiman_reconnect_attempted_to_json(bool attempted);

cJSON* pres_http_dto_wifiman_accepted_to_json(void);

cJSON* pres_http_dto_wifiman_forgotten_to_json(void);

#ifdef __cplusplus
}
#endif

#endif /* PRESENTATION_HTTP_DTO_WIFIMAN_H */
