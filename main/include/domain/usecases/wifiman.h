#ifndef DOMAIN_USECASES_WIFIMAN_H
#define DOMAIN_USECASES_WIFIMAN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "domain/models/error.h"
#include "domain/models/network.h"
#include "domain/models/wifi.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dom_usecases_wifiman_t dom_usecases_wifiman_t;

typedef struct {
    bool available;
    char ssid[DOM_MODELS_WIFI_SSID_BUF_LEN];
} dom_usecases_wifiman_stored_sta_t;

typedef struct {
    dom_models_wifi_status_t              wifi;
    bool                                  sta_netif_available;
    dom_models_network_interface_t        sta_netif;
    dom_usecases_wifiman_stored_sta_t     stored_sta;
    bool                                  auto_reconnect_enabled;
    size_t                                reconnect_trial_count;
    size_t                                reconnect_max_trials;
    bool                                  ap_auto_manage_enabled;
    bool                                  sta_connection_commit_required;
} dom_usecases_wifiman_status_t;

struct dom_usecases_wifiman_t {
    void* ctx;
    dom_models_error_t (*start)(
        dom_usecases_wifiman_t* self
    );
    dom_models_error_t (*stop)(
        dom_usecases_wifiman_t* self
    );
    dom_models_error_t (*start_scan)(
        dom_usecases_wifiman_t*              self,
        const dom_models_wifi_scan_config_t* config
    );
    dom_models_error_t (*get_scan_result)(
        dom_usecases_wifiman_t*        self,
        dom_models_wifi_scan_result_t* out
    );
    dom_models_error_t (*get_status)(
        dom_usecases_wifiman_t*        self,
        dom_usecases_wifiman_status_t* out
    );
    dom_models_error_t (*connect_sta)(
        dom_usecases_wifiman_t*                  self,
        const dom_models_wifi_sta_credential_t* credential
    );
    dom_models_error_t (*connect_stored_sta)(
        dom_usecases_wifiman_t* self
    );
    dom_models_error_t (*disconnect_sta)(
        dom_usecases_wifiman_t* self
    );
    dom_models_error_t (*commit_sta_connection)(
        dom_usecases_wifiman_t* self
    );
    dom_models_error_t (*get_stored_sta)(
        dom_usecases_wifiman_t*            self,
        dom_usecases_wifiman_stored_sta_t* out
    );
    dom_models_error_t (*set_sta_credential)(
        dom_usecases_wifiman_t*                  self,
        const dom_models_wifi_sta_credential_t* credential
    );
    dom_models_error_t (*forget_sta_credential)(
        dom_usecases_wifiman_t* self
    );
    dom_models_error_t (*need_reconnect)(
        dom_usecases_wifiman_t* self,
        bool*                   out
    );
    dom_models_error_t (*try_reconnect)(
        dom_usecases_wifiman_t* self,
        bool*                   attempted
    );
};

static inline dom_usecases_wifiman_t* dom_usecases_wifiman_new(void* ctx) {
    dom_usecases_wifiman_t* self = (dom_usecases_wifiman_t*)calloc(1, sizeof(dom_usecases_wifiman_t));
    if (!self) {
        return NULL;
    }

    self->ctx = ctx;

    return self;
}

static inline void dom_usecases_wifiman_delete(dom_usecases_wifiman_t* self) {
    if (!self) {
        return;
    }

    self->ctx = NULL;
    free(self);
}

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_USECASES_WIFIMAN_H */
