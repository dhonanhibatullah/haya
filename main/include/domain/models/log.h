#ifndef DOMAIN_MODELS_LOG_H
#define DOMAIN_MODELS_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DOMAIN_MODELS_LOG_LEVEL_NONE,
    DOMAIN_MODELS_LOG_LEVEL_ERROR,
    DOMAIN_MODELS_LOG_LEVEL_WARN,
    DOMAIN_MODELS_LOG_LEVEL_INFO,
    DOMAIN_MODELS_LOG_LEVEL_DEBUG,
} dom_models_log_level_t;

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_MODELS_LOG_H */