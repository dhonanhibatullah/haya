#ifndef INFRASTRUCTURE_LOGGING_LEVELED_STDIO_IMPL_H
#define INFRASTRUCTURE_LOGGING_LEVELED_STDIO_IMPL_H

#include <stddef.h>

#include "domain/contracts/logger/leveled.h"
#include "domain/models/logger.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    dom_models_logger_level_t level;
    unsigned int              cb_max_cnt;
} inf_logger_leveled_stdio_impl_cfg_t;

#define INF_LOGGER_LEVELED_STDIO_IMPL_CFG_DEFAULT()    \
    {                                                  \
        .level      = DOMAIN_MODELS_LOGGER_LEVEL_INFO, \
        .cb_max_cnt = 0,                               \
    }

typedef struct {
    inf_logger_leveled_stdio_impl_cfg_t cfg;
    dom_contracts_logger_leveled_cb*    cb_funcs;
    void**                              cb_ctxs;
    unsigned int                        cb_idx;
} inf_logger_leveled_stdio_impl_ctx_t;

dom_contracts_logger_leveled_t* inf_logger_leveled_stdio_impl_new(const inf_logger_leveled_stdio_impl_cfg_t* cfg);

void inf_logger_leveled_stdio_impl_delete(dom_contracts_logger_leveled_t* self);

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_LOGGING_LEVELED_STDIO_IMPL_H */