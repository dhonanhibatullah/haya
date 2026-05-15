#ifndef DOMAIN_PORTS_OUTBOUND_LOGGING_LEVELED_H
#define DOMAIN_PORTS_OUTBOUND_LOGGING_LEVELED_H

#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct dpo_logging_leveled_s dpo_logging_leveled_t;

struct dpo_logging_leveled_s {
    void* ctx;
    void (*error)(
        dpo_logging_leveled_t* self,
        const char*            tag,
        const char*            format,
        ...
    );
    void (*warn)(
        dpo_logging_leveled_t* self,
        const char*            tag,
        const char*            format,
        ...
    );
    void (*info)(
        dpo_logging_leveled_t* self,
        const char*            tag,
        const char*            format,
        ...
    );
    void (*debug)(
        dpo_logging_leveled_t* self,
        const char*            tag,
        const char*            format,
        ...
    );
};

inline dpo_logging_leveled_t* dpo_logging_leveled_new(void* ctx) {
    dpo_logging_leveled_t* self = calloc(1, sizeof(dpo_logging_leveled_t));
    if (!self) {
        return NULL;
    }
    self->ctx = ctx;
    return self;
}

inline void dpo_logging_leveled_delete(dpo_logging_leveled_t* self) {
    free(self);
}

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_PORTS_OUTBOUND_LOGGING_LEVELED_H */