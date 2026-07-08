#ifndef DOMAIN_MODELS_ERROR_H
#define DOMAIN_MODELS_ERROR_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DOMAIN_MODELS_ERROR_TYPE(X)                              \
    X(DOMAIN_MODELS_ERROR_TYPE_OK, "OK")                         \
    X(DOMAIN_MODELS_ERROR_TYPE_UNIMPLEMENTED, "UNIMPLEMENTED")   \
    X(DOMAIN_MODELS_ERROR_TYPE_NOT_SUPPORTED, "NOT_SUPPORTED")   \
    X(DOMAIN_MODELS_ERROR_TYPE_MALLOC_FAILED, "MALLOC_FAILED")   \
    X(DOMAIN_MODELS_ERROR_TYPE_BAD_ARGUMENT, "INVALID_ARGUMENT") \
    X(DOMAIN_MODELS_ERROR_TYPE_BAD_STATE, "BAD_STATE")           \
    X(DOMAIN_MODELS_ERROR_TYPE_TIMEOUT, "TIMEOUT")               \
    X(DOMAIN_MODELS_ERROR_TYPE_SYSTEM_FAILURE, "SYSTEM_FAILURE") \
    X(DOMAIN_MODELS_ERROR_TYPE_UNKNOWN, "UNKNOWN")

typedef enum {
#define X(cb_name, cb_string) cb_name,
    DOMAIN_MODELS_ERROR_TYPE(X)
#undef X
} dom_models_error_type_t;

static inline const char* dom_models_error_type_str(dom_models_error_type_t err) {
    switch (err) {
#define X(cb_name, cb_string) \
    case cb_name:             \
        return cb_string;
        DOMAIN_MODELS_ERROR_TYPE(X)
#undef X
    }
    return "UNKNOWN";
}

typedef struct dom_models_error_t dom_models_error_t;

struct dom_models_error_t {
    char                    actor[64];
    char*                   msg;
    size_t                  len;
    dom_models_error_type_t type;
    dom_models_error_t*     prev;
};

dom_models_error_t* dom_models_error_new(const char* actor, dom_models_error_type_t type, const char* fmt, ...);

void dom_models_error_delete(dom_models_error_t* handle);

dom_models_error_t* dom_models_error_append(dom_models_error_t* handle, const char* actor, dom_models_error_type_t type, const char* fmt, ...);

size_t dom_models_error_total_length(const dom_models_error_t* handle);

size_t dom_models_error_print(const dom_models_error_t* handle, char* buf, size_t buf_size);

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_MODELS_ERROR_H */
