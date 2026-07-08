#include "domain/contracts/device/ethernet.h"

#include <stdlib.h>

dom_contracts_device_ethernet_t* dom_contracts_device_ethernet_new(void* ctx) {
    dom_contracts_device_ethernet_t* self = (dom_contracts_device_ethernet_t*)calloc(1, sizeof(dom_contracts_device_ethernet_t));
    if (!self) {
        return NULL;
    }

    self->ctx = ctx;

    return self;
}

void dom_contracts_device_ethernet_delete(dom_contracts_device_ethernet_t* self) {
    if (!self) {
        return;
    }

    self->ctx = NULL;
    free(self);
}
