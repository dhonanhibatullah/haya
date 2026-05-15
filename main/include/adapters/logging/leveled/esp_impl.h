#ifndef ADAPTERS_LOGGING_LEVELED_ESP_IMPL_H
#define ADAPTERS_LOGGING_LEVELED_ESP_IMPL_H

#include "domain/ports/outbound/logging/leveled.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
} adp_logging_leveled_esp_impl_cfg_t;

typedef struct {
    adp_logging_leveled_esp_impl_cfg_t cfg;
} adp_logging_leveled_esp_impl_ctx_t;

dpo_logging_leveled_t* adp_logging_leveled_esp_impl_new(const adp_logging_leveled_esp_impl_cfg_t* cfg);

void adp_logging_leveled_esp_impl_delete(dpo_logging_leveled_t* self);

#ifdef __cplusplus
}
#endif

#endif /* ADAPTERS_LOGGING_LEVELED_ESP_IMPL_H */