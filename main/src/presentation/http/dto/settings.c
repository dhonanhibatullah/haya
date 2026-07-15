#include "presentation/http/dto/settings.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "cJSON.h"
#include "domain/models/error.h"
#include "domain/models/system.h"
#include "domain/usecases/settings.h"

/* Helper Function Prototypes */

static size_t bounded_strlen(const char* value, size_t max_len);
static dom_models_error_t copy_optional_json_string(cJSON* json, const char* key, char* out, size_t out_size, bool* set);
static dom_models_error_t copy_optional_json_uint32(cJSON* json, const char* key, uint32_t* out, bool* set);
static cJSON* preloaded_to_json(const dom_usecases_settings_snapshot_t* snapshot);
static cJSON* project_to_json(const dom_models_system_project_info_t* project);
static cJSON* chip_to_json(const dom_models_system_chip_info_t* chip);

dom_models_error_t pres_http_dto_settings_parse_preloaded_update(
    cJSON*                                    json,
    dom_usecases_settings_preloaded_update_t* out
) {
    dom_models_error_t err = DOMAIN_MODELS_ERROR_OK;

    if (!json || !out || !cJSON_IsObject(json)) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    memset(out, 0, sizeof(dom_usecases_settings_preloaded_update_t));

    err = copy_optional_json_string(json, "wifi_ap_ssid", out->wifi_ap_ssid, sizeof(out->wifi_ap_ssid), &out->wifi_ap_ssid_set);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    err = copy_optional_json_string(json, "wifi_ap_pass", out->wifi_ap_pass, sizeof(out->wifi_ap_pass), &out->wifi_ap_pass_set);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    err = copy_optional_json_string(json, "mqtt_proto", out->mqtt_proto, sizeof(out->mqtt_proto), &out->mqtt_proto_set);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    err = copy_optional_json_string(json, "mqtt_host", out->mqtt_host, sizeof(out->mqtt_host), &out->mqtt_host_set);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    err = copy_optional_json_string(json, "mqtt_port", out->mqtt_port, sizeof(out->mqtt_port), &out->mqtt_port_set);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    err = copy_optional_json_string(json, "mqtt_user", out->mqtt_user, sizeof(out->mqtt_user), &out->mqtt_user_set);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    err = copy_optional_json_string(json, "mqtt_pass", out->mqtt_pass, sizeof(out->mqtt_pass), &out->mqtt_pass_set);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    return copy_optional_json_uint32(json, "system_restart_after_ms", &out->system_restart_after_ms, &out->system_restart_after_ms_set);
}

cJSON* pres_http_dto_settings_snapshot_to_json(const dom_usecases_settings_snapshot_t* snapshot) {
    if (!snapshot) {
        return NULL;
    }

    cJSON* root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    cJSON_AddNumberToObject(root, "device_id", (double)snapshot->device_id);
    cJSON_AddStringToObject(root, "device_id_str", snapshot->device_id_str);
    cJSON_AddItemToObject(root, "preloaded", preloaded_to_json(snapshot));
    cJSON_AddItemToObject(root, "project", project_to_json(&snapshot->project));
    cJSON_AddItemToObject(root, "chip", chip_to_json(&snapshot->chip));
    cJSON_AddBoolToObject(root, "restart_required", snapshot->restart_required);

    return root;
}

cJSON* pres_http_dto_settings_preloaded_updated_to_json(bool restart_required) {
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    cJSON_AddBoolToObject(root, "updated", true);
    cJSON_AddBoolToObject(root, "restart_required", restart_required);

    return root;
}

cJSON* pres_http_dto_settings_restart_required_to_json(bool restart_required) {
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    cJSON_AddBoolToObject(root, "restart_required", restart_required);

    return root;
}

cJSON* pres_http_dto_settings_accepted_to_json(void) {
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    cJSON_AddBoolToObject(root, "accepted", true);

    return root;
}

/* Helper Function Implementations */

static size_t bounded_strlen(const char* value, size_t max_len) {
    size_t len = 0;

    if (!value) {
        return 0;
    }

    while (len < max_len && value[len] != '\0') {
        len++;
    }

    return len;
}

static dom_models_error_t copy_optional_json_string(cJSON* json, const char* key, char* out, size_t out_size, bool* set) {
    if (!json || !key || !out || out_size == 0 || !set) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    cJSON* value = cJSON_GetObjectItemCaseSensitive(json, key);
    if (!value) {
        return DOMAIN_MODELS_ERROR_OK;
    }
    if (!cJSON_IsString(value) || !value->valuestring) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    size_t len = bounded_strlen(value->valuestring, out_size);
    if (len >= out_size) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    memcpy(out, value->valuestring, len);
    out[len] = '\0';
    *set     = true;

    return DOMAIN_MODELS_ERROR_OK;
}

static cJSON* preloaded_to_json(const dom_usecases_settings_snapshot_t* snapshot) {
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, "wifi_ap_ssid", snapshot->wifi_ap_ssid);
    cJSON_AddStringToObject(root, "wifi_ap_pass", snapshot->wifi_ap_pass);
    cJSON_AddStringToObject(root, "mqtt_proto", snapshot->mqtt_proto);
    cJSON_AddStringToObject(root, "mqtt_host", snapshot->mqtt_host);
    cJSON_AddStringToObject(root, "mqtt_port", snapshot->mqtt_port);
    cJSON_AddStringToObject(root, "mqtt_user", snapshot->mqtt_user);
    cJSON_AddStringToObject(root, "mqtt_pass", snapshot->mqtt_pass);
    cJSON_AddNumberToObject(root, "system_restart_after_ms", snapshot->system_restart_after_ms);

    return root;
}

static cJSON* project_to_json(const dom_models_system_project_info_t* project) {
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, "project_name", project->project_name);
    cJSON_AddStringToObject(root, "project_version", project->project_version);
    cJSON_AddStringToObject(root, "name", project->name);
    cJSON_AddStringToObject(root, "type", project->type);
    cJSON_AddStringToObject(root, "firmware_version", project->firmware_version);

    return root;
}

static cJSON* chip_to_json(const dom_models_system_chip_info_t* chip) {
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, "hardware_mac", chip->hardware_mac);
    cJSON_AddStringToObject(root, "model", chip->model);
    cJSON_AddNumberToObject(root, "revision", chip->revision);
    cJSON_AddNumberToObject(root, "cores", chip->cores);

    return root;
}

static dom_models_error_t copy_optional_json_uint32(cJSON* json, const char* key, uint32_t* out, bool* set) {
    if (!json || !key || !out || !set) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    cJSON* value = cJSON_GetObjectItemCaseSensitive(json, key);
    if (!value) {
        return DOMAIN_MODELS_ERROR_OK;
    }
    if (!cJSON_IsNumber(value)) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    *out = (uint32_t)value->valuedouble;
    *set = true;

    return DOMAIN_MODELS_ERROR_OK;
}
