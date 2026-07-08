#ifndef ADAPTERS_LOGGING_LEVELED_BLE_IMPL_H
#define ADAPTERS_LOGGING_LEVELED_BLE_IMPL_H

#include "domain/models/log.h"
#include "domain/ports/outbound/logging/leveled.h"
#include "esp_err.h"
#include "host/ble_gatt.h"
#include "host/ble_uuid.h"

#define ADAPTERS_LOGGING_LEVELED_BLE_IMPL_MESSAGE_MAX_LEN 244

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ble_gatt_svc_def adp_logging_leveled_ble_impl_svc_def_t;
typedef struct ble_gatt_chr_def adp_logging_leveled_ble_impl_chr_def_t;

typedef struct {
    dom_models_log_level_t level;
    ble_uuid128_t          service_uuid;
    ble_uuid128_t          toggle_characteristic_uuid;
    ble_uuid128_t          message_characteristic_uuid;
} adp_logging_leveled_ble_impl_cfg_t;

typedef struct {
    adp_logging_leveled_ble_impl_cfg_t cfg;

    adp_logging_leveled_ble_impl_svc_def_t service_defs[2];
    adp_logging_leveled_ble_impl_chr_def_t characteristic_defs[3];

    uint16_t toggle_chr_hdl;
    uint16_t message_chr_hdl;
    bool     initiated;
    bool     enabled;

    char     message[ADAPTERS_LOGGING_LEVELED_BLE_IMPL_MESSAGE_MAX_LEN];
    uint16_t message_len;
} adp_logging_leveled_ble_impl_ctx_t;

dpo_logging_leveled_t* adp_logging_leveled_ble_impl_new(const adp_logging_leveled_ble_impl_cfg_t* cfg);

void adp_logging_leveled_ble_impl_delete(dpo_logging_leveled_t* self);

esp_err_t adp_logging_leveled_ble_impl_init(dpo_logging_leveled_t* self);

#ifdef __cplusplus
}
#endif

#endif /* ADAPTERS_LOGGING_LEVELED_BLE_IMPL_H */