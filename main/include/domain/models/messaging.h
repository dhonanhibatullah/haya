#ifndef DOMAIN_MODELS_MESSAGING_H
#define DOMAIN_MODELS_MESSAGING_H

#include "domain/models/system.h"
#include "domain/models/update.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DOM_MODELS_MESSAGING_TOPIC_MAX_LEN       96
#define DOM_MODELS_MESSAGING_NAME_MAX_LEN        64
#define DOM_MODELS_MESSAGING_TYPE_MAX_LEN        32
#define DOM_MODELS_MESSAGING_DEVICE_ID_MAX_LEN   37
#define DOM_MODELS_MESSAGING_SESSION_KEY_MAX_LEN 96
#define DOM_MODELS_MESSAGING_SIGNATURE_MAX_LEN   129
#define DOM_MODELS_MESSAGING_STATUS_MAX_LEN      16
#define DOM_MODELS_MESSAGING_LOG_MAX_LEN         256

typedef enum {
    DOM_MODELS_MESSAGING_TOPIC_DIRECTION_PUB = 0,
    DOM_MODELS_MESSAGING_TOPIC_DIRECTION_SUB,
} dom_models_messaging_topic_direction_t;

typedef struct {
    char hardware_mac[DOM_MODELS_SYSTEM_HARDWARE_MAC_MAX_LEN];
    char name[DOM_MODELS_MESSAGING_NAME_MAX_LEN];
    char type[DOM_MODELS_MESSAGING_TYPE_MAX_LEN];
    char firmware_version[DOM_MODELS_SYSTEM_FIRMWARE_VERSION_MAX_LEN];
    char session_key[DOM_MODELS_MESSAGING_SESSION_KEY_MAX_LEN];
    char signature[DOM_MODELS_MESSAGING_SIGNATURE_MAX_LEN];
} dom_models_messaging_registration_t;

typedef struct {
    char status[DOM_MODELS_MESSAGING_STATUS_MAX_LEN];
} dom_models_messaging_status_t;

typedef struct {
    char device_id[DOM_MODELS_MESSAGING_DEVICE_ID_MAX_LEN];
} dom_models_messaging_registration_ack_t;

typedef struct {
    char message[DOM_MODELS_MESSAGING_LOG_MAX_LEN];
} dom_models_messaging_log_t;

typedef struct {
    dom_models_update_info_t update_info;
} dom_models_messaging_update_t;

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_MODELS_MESSAGING_H */
