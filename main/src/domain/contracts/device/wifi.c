#include "domain/contracts/device/wifi.h"

#include <stdlib.h>

dom_contracts_device_wifi_t* dom_contracts_device_wifi_new(void* ctx) {
    dom_contracts_device_wifi_t* self = (dom_contracts_device_wifi_t*)calloc(1, sizeof(dom_contracts_device_wifi_t));
    if (!self) {
        return NULL;
    }

    self->ctx = ctx;

    return self;
}

void dom_contracts_device_wifi_delete(dom_contracts_device_wifi_t* self) {
    if (!self) {
        return;
    }

    self->ctx = NULL;
    free(self);
}
