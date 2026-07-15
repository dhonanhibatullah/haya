#ifndef DOMAIN_MODELS_SYSTEM_H
#define DOMAIN_MODELS_SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

#define DOM_MODELS_SYSTEM_HARDWARE_MAC_MAX_LEN     18
#define DOM_MODELS_SYSTEM_DEVICE_NAME_MAX_LEN      64
#define DOM_MODELS_SYSTEM_DEVICE_TYPE_MAX_LEN      32
#define DOM_MODELS_SYSTEM_FIRMWARE_VERSION_MAX_LEN 48
#define DOM_MODELS_SYSTEM_PROJECT_NAME_MAX_LEN     64
#define DOM_MODELS_SYSTEM_PROJECT_VERSION_MAX_LEN  48
#define DOM_MODELS_SYSTEM_CHIP_MODEL_MAX_LEN       32
#define DOM_MODELS_SYSTEM_AUTH_SESSION_KEY_MAX_LEN 96
#define DOM_MODELS_SYSTEM_AUTH_SIGNATURE_MAX_LEN   129

typedef struct {
    char project_name[DOM_MODELS_SYSTEM_PROJECT_NAME_MAX_LEN];
    char project_version[DOM_MODELS_SYSTEM_PROJECT_VERSION_MAX_LEN];
    char name[DOM_MODELS_SYSTEM_DEVICE_NAME_MAX_LEN];
    char type[DOM_MODELS_SYSTEM_DEVICE_TYPE_MAX_LEN];
    char firmware_version[DOM_MODELS_SYSTEM_FIRMWARE_VERSION_MAX_LEN];
} dom_models_system_project_info_t;

typedef struct {
    char hardware_mac[DOM_MODELS_SYSTEM_HARDWARE_MAC_MAX_LEN];
    char model[DOM_MODELS_SYSTEM_CHIP_MODEL_MAX_LEN];
    int  revision;
    int  cores;
} dom_models_system_chip_info_t;

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_MODELS_SYSTEM_H */
