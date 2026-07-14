#ifndef DOMAIN_CONTRACTS_DEVICE_WIFI_H
#define DOMAIN_CONTRACTS_DEVICE_WIFI_H

#include <stdlib.h>

#include "domain/models/error.h"
#include "domain/models/wifi.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dom_contracts_device_wifi_t dom_contracts_device_wifi_t;

struct dom_contracts_device_wifi_t {
    void* ctx;
    dom_models_error_t (*start)(
        dom_contracts_device_wifi_t* self
    );
    dom_models_error_t (*stop)(
        dom_contracts_device_wifi_t* self
    );
    dom_models_error_t (*set_mode)(
        dom_contracts_device_wifi_t* self,
        dom_models_wifi_mode_t       mode
    );
    dom_models_error_t (*get_status)(
        dom_contracts_device_wifi_t* self,
        dom_models_wifi_status_t*    out
    );
    /* Starts a station connection attempt and returns after the driver accepts the request. */
    dom_models_error_t (*connect_sta)(
        dom_contracts_device_wifi_t*                self,
        const dom_models_wifi_sta_connect_config_t* config
    );
    dom_models_error_t (*disconnect_sta)(
        dom_contracts_device_wifi_t* self
    );
    dom_models_error_t (*start_ap)(
        dom_contracts_device_wifi_t*       self,
        const dom_models_wifi_ap_config_t* config
    );
    dom_models_error_t (*stop_ap)(
        dom_contracts_device_wifi_t* self
    );
    /* Starts a scan request; use get_scanned() to observe completion and results. */
    dom_models_error_t (*start_scan)(
        dom_contracts_device_wifi_t*         self,
        const dom_models_wifi_scan_config_t* config
    );
    dom_models_error_t (*get_scanned)(
        dom_contracts_device_wifi_t*   self,
        dom_models_wifi_scan_result_t* out
    );
};

static inline dom_contracts_device_wifi_t* dom_contracts_device_wifi_new(void* ctx) {
    dom_contracts_device_wifi_t* self = (dom_contracts_device_wifi_t*)calloc(1, sizeof(dom_contracts_device_wifi_t));
    if (!self) {
        return NULL;
    }

    self->ctx = ctx;

    return self;
}

static inline void dom_contracts_device_wifi_delete(dom_contracts_device_wifi_t* self) {
    if (!self) {
        return;
    }

    self->ctx = NULL;
    free(self);
}

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_CONTRACTS_DEVICE_WIFI_H */
