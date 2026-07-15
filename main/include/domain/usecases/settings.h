#ifndef DOMAIN_USECASES_SETTINGS_H
#define DOMAIN_USECASES_SETTINGS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "domain/models/error.h"
#include "domain/models/system.h"
#include "domain/models/wifi.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DOM_USECASES_SETTINGS_DEVICE_ID_STR_MAX_LEN 32
#define DOM_USECASES_SETTINGS_MQTT_PROTO_MAX_LEN    16
#define DOM_USECASES_SETTINGS_MQTT_HOST_MAX_LEN     128
#define DOM_USECASES_SETTINGS_MQTT_PORT_MAX_LEN     8
#define DOM_USECASES_SETTINGS_MQTT_USER_MAX_LEN     64
#define DOM_USECASES_SETTINGS_MQTT_PASS_MAX_LEN     128

typedef struct {
    uint64_t device_id;
    char     device_id_str[DOM_USECASES_SETTINGS_DEVICE_ID_STR_MAX_LEN];

    char wifi_ap_ssid[DOM_MODELS_WIFI_SSID_BUF_LEN];
    char wifi_ap_pass[DOM_MODELS_WIFI_PASSWORD_BUF_LEN];

    char mqtt_proto[DOM_USECASES_SETTINGS_MQTT_PROTO_MAX_LEN];
    char mqtt_host[DOM_USECASES_SETTINGS_MQTT_HOST_MAX_LEN];
    char mqtt_port[DOM_USECASES_SETTINGS_MQTT_PORT_MAX_LEN];
    char mqtt_user[DOM_USECASES_SETTINGS_MQTT_USER_MAX_LEN];
    char mqtt_pass[DOM_USECASES_SETTINGS_MQTT_PASS_MAX_LEN];

    dom_models_system_project_info_t project;
    dom_models_system_chip_info_t    chip;

    bool restart_required;
} dom_usecases_settings_snapshot_t;

typedef struct {
    bool wifi_ap_ssid_set;
    char wifi_ap_ssid[DOM_MODELS_WIFI_SSID_BUF_LEN];

    bool wifi_ap_pass_set;
    char wifi_ap_pass[DOM_MODELS_WIFI_PASSWORD_BUF_LEN];

    bool mqtt_proto_set;
    char mqtt_proto[DOM_USECASES_SETTINGS_MQTT_PROTO_MAX_LEN];

    bool mqtt_host_set;
    char mqtt_host[DOM_USECASES_SETTINGS_MQTT_HOST_MAX_LEN];

    bool mqtt_port_set;
    char mqtt_port[DOM_USECASES_SETTINGS_MQTT_PORT_MAX_LEN];

    bool mqtt_user_set;
    char mqtt_user[DOM_USECASES_SETTINGS_MQTT_USER_MAX_LEN];

    bool mqtt_pass_set;
    char mqtt_pass[DOM_USECASES_SETTINGS_MQTT_PASS_MAX_LEN];
} dom_usecases_settings_preloaded_update_t;

typedef struct dom_usecases_settings_t dom_usecases_settings_t;

struct dom_usecases_settings_t {
    void* ctx;
    dom_models_error_t (*get_snapshot)(
        dom_usecases_settings_t*          self,
        dom_usecases_settings_snapshot_t* out
    );
    dom_models_error_t (*set_preloaded)(
        dom_usecases_settings_t*                         self,
        const dom_usecases_settings_preloaded_update_t*  update,
        bool*                                            restart_required_out
    );
    dom_models_error_t (*get_restart_required)(
        dom_usecases_settings_t* self,
        bool*                    out
    );
    dom_models_error_t (*restart)(
        dom_usecases_settings_t* self,
        uint32_t                 delay_ms
    );
};

static inline dom_usecases_settings_t* dom_usecases_settings_new(void* ctx) {
    dom_usecases_settings_t* self = (dom_usecases_settings_t*)calloc(1, sizeof(dom_usecases_settings_t));
    if (!self) {
        return NULL;
    }

    self->ctx = ctx;

    return self;
}

static inline void dom_usecases_settings_delete(dom_usecases_settings_t* self) {
    if (!self) {
        return;
    }

    self->ctx = NULL;
    free(self);
}

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_USECASES_SETTINGS_H */
