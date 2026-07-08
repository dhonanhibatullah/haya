#ifndef DOMAIN_CONTRACTS_LOGGER_LEVELED_H
#define DOMAIN_CONTRACTS_LOGGER_LEVELED_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dom_contracts_logger_leveled_t dom_contracts_logger_leveled_t;

typedef void (*dom_contracts_logger_leveled_cb)(void* cb_ctx, const char* msg, size_t msg_len);

struct dom_contracts_logger_leveled_t {
    void* ctx;
    void (*error)(
        dom_contracts_logger_leveled_t* self,
        const char*                     tag,
        const char*                     format,
        ...
    );
    void (*warn)(
        dom_contracts_logger_leveled_t* self,
        const char*                     tag,
        const char*                     format,
        ...
    );
    void (*info)(
        dom_contracts_logger_leveled_t* self,
        const char*                     tag,
        const char*                     format,
        ...
    );
    void (*debug)(
        dom_contracts_logger_leveled_t* self,
        const char*                     tag,
        const char*                     format,
        ...
    );
    void (*add_callback)(
        dom_contracts_logger_leveled_t* self,
        void*                           cb_ctx,
        dom_contracts_logger_leveled_cb cb_func
    );
};

dom_contracts_logger_leveled_t* dom_contracts_logger_leveled_new(void* ctx);

void dom_contracts_logger_leveled_delete(dom_contracts_logger_leveled_t* self);

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_CONTRACTS_LOGGER_LEVELED_H */
