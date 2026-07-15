#ifndef DOMAIN_CONTRACTS_MESSAGING_PUBLISH_H
#define DOMAIN_CONTRACTS_MESSAGING_PUBLISH_H

#include <stdbool.h>
#include <stdlib.h>

#include "domain/models/error.h"
#include "domain/models/messaging.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dom_contracts_messaging_publish_t dom_contracts_messaging_publish_t;

struct dom_contracts_messaging_publish_t {
    void* ctx;
    dom_models_error_t (*send_registration)(
        dom_contracts_messaging_publish_t*         self,
        const dom_models_messaging_registration_t* registration
    );
    dom_models_error_t (*send_status)(
        dom_contracts_messaging_publish_t*   self,
        const dom_models_messaging_status_t* status
    );
    dom_models_error_t (*send_log)(
        dom_contracts_messaging_publish_t* self,
        const dom_models_messaging_log_t*  log
    );
    dom_models_error_t (*is_connected)(
        dom_contracts_messaging_publish_t* self,
        bool*                              out
    );
};

static inline dom_contracts_messaging_publish_t* dom_contracts_messaging_publish_new(void* ctx) {
    dom_contracts_messaging_publish_t* self = (dom_contracts_messaging_publish_t*)calloc(1, sizeof(dom_contracts_messaging_publish_t));
    if (!self) {
        return NULL;
    }

    self->ctx = ctx;

    return self;
}

static inline void dom_contracts_messaging_publish_delete(dom_contracts_messaging_publish_t* self) {
    if (!self) {
        return;
    }

    self->ctx = NULL;
    free(self);
}

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_CONTRACTS_MESSAGING_PUBLISH_H */
