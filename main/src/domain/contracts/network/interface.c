#include "domain/contracts/network/interface.h"

#include <stdlib.h>

dom_contracts_network_interface_t* dom_contracts_network_interface_new(void* ctx) {
    dom_contracts_network_interface_t* self = (dom_contracts_network_interface_t*)calloc(1, sizeof(dom_contracts_network_interface_t));
    if (!self) {
        return NULL;
    }

    self->ctx = ctx;

    return self;
}

void dom_contracts_network_interface_delete(dom_contracts_network_interface_t* self) {
    if (!self) {
        return;
    }

    self->ctx = NULL;
    free(self);
}
