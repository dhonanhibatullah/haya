#ifndef DOMAIN_MODELS_ERROR_H
#define DOMAIN_MODELS_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

#define DOMAIN_MODEL_ERROR(X)                                   \
    X(DOMAIN_MODELS_ERROR_OK, "Ok")                             \
    X(DOMAIN_MODELS_ERROR_UNIMPLEMENTED, "Unimplemented")       \
    X(DOMAIN_MODELS_ERROR_INVALID_ARGUMENT, "Invalid argument") \
    X(DOMAIN_MODELS_ERROR_MALLOC, "Memory allocation failed")   \
    X(DOMAIN_MODELS_ERROR_BAD_STATE, "Bad state")               \
    X(DOMAIN_MODELS_ERROR_FAIL, "System failure")               \
    X(DOMAIN_MODELS_ERROR_UNKNOWN, "Unknown error")

typedef enum {
#define X(cb_name, cb_string) cb_name,
    DOMAIN_MODEL_ERROR(X)
#undef X
} dom_models_error_t;

static inline const char* err_str(dom_models_error_t err) {
    switch (err) {
#define X(cb_name, cb_string) \
    case cb_name:             \
        return cb_string;
        DOMAIN_MODEL_ERROR(X)
#undef X
    }
    return "Unknown error";
}

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_MODELS_ERROR_H */