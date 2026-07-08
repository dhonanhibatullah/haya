#ifndef DOMAIN_CONTRACTS_DEVICE_WIFI_H
#define DOMAIN_CONTRACTS_DEVICE_WIFI_H

#include "domain/models/error.h"
#include "domain/models/wifi.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dom_contracts_device_wifi_t dom_contracts_device_wifi_t;

struct dom_contracts_device_wifi_t {
    void* ctx;
    dom_models_error_t* (*start)(
        dom_contracts_device_wifi_t* self
    );
    dom_models_error_t* (*stop)(
        dom_contracts_device_wifi_t* self
    );
    dom_models_error_t* (*set_mode)(
        dom_contracts_device_wifi_t* self,
        dom_models_wifi_mode_t       mode
    );
    dom_models_error_t* (*get_status)(
        dom_contracts_device_wifi_t* self,
        dom_models_wifi_status_t*    out
    );
    dom_models_error_t* (*connect_sta)(
        dom_contracts_device_wifi_t*                self,
        const dom_models_wifi_sta_connect_config_t* config
    );
    dom_models_error_t* (*disconnect_sta)(
        dom_contracts_device_wifi_t* self
    );
    dom_models_error_t* (*start_ap)(
        dom_contracts_device_wifi_t*       self,
        const dom_models_wifi_ap_config_t* config
    );
    dom_models_error_t* (*stop_ap)(
        dom_contracts_device_wifi_t* self
    );
    dom_models_error_t* (*scan)(
        dom_contracts_device_wifi_t*         self,
        const dom_models_wifi_scan_config_t* config,
        dom_models_wifi_scan_result_t*       out
    );
};

dom_contracts_device_wifi_t* dom_contracts_device_wifi_new(void* ctx);

void dom_contracts_device_wifi_delete(dom_contracts_device_wifi_t* self);

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_CONTRACTS_DEVICE_WIFI_H */
