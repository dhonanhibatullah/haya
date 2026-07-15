#ifndef DOMAIN_CONTRACTS_MESSAGING_SUBSCRIBE_H
#define DOMAIN_CONTRACTS_MESSAGING_SUBSCRIBE_H

#include <stdlib.h>

#include "domain/models/error.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dom_contracts_messaging_subscribe_t dom_contracts_messaging_subscribe_t;

struct dom_contracts_messaging_subscribe_t {
    void* ctx;
    dom_models_error_t (*subscribe_registration_ack)(
        dom_contracts_messaging_subscribe_t* self
    );
    dom_models_error_t (*subscribe_update)(
        dom_contracts_messaging_subscribe_t* self
    );
    dom_models_error_t (*subscribe_restart)(
        dom_contracts_messaging_subscribe_t* self
    );
};

static inline dom_contracts_messaging_subscribe_t* dom_contracts_messaging_subscribe_new(void* ctx) {
    dom_contracts_messaging_subscribe_t* self = (dom_contracts_messaging_subscribe_t*)calloc(1, sizeof(dom_contracts_messaging_subscribe_t));
    if (!self) {
        return NULL;
    }

    self->ctx = ctx;

    return self;
}

static inline void dom_contracts_messaging_subscribe_delete(dom_contracts_messaging_subscribe_t* self) {
    if (!self) {
        return;
    }

    self->ctx = NULL;
    free(self);
}

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_CONTRACTS_MESSAGING_SUBSCRIBE_H */
