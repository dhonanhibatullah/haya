#include "domain/contracts/logger/leveled.h"

#include <stdlib.h>

dom_contracts_logger_leveled_t* dom_contracts_logger_leveled_new(void* ctx) {
    dom_contracts_logger_leveled_t* self = (dom_contracts_logger_leveled_t*)calloc(1, sizeof(dom_contracts_logger_leveled_t));
    if (!self) {
        return NULL;
    }
    self->ctx = ctx;
    return self;
}

void dom_contracts_logger_leveled_delete(dom_contracts_logger_leveled_t* self) {
    self->ctx = NULL;
    free(self);
}