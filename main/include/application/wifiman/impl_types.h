#ifndef APPLICATION_WIFIMAN_IMPL_TYPES_H
#define APPLICATION_WIFIMAN_IMPL_TYPES_H

#include <stdbool.h>
#include <stddef.h>

#include "domain/contracts/device/wifi.h"
#include "domain/contracts/logger/leveled.h"
#include "domain/contracts/network/interface.h"
#include "domain/contracts/repository/preloaded.h"
#include "domain/contracts/repository/wifi.h"

#ifdef __cplusplus
extern "C" {
#endif

#define APP_WIFIMAN_IMPL_DEFAULT_RECONNECT_MAX_TRIALS 5

typedef enum {
    APP_WIFIMAN_IMPL_STA_CONNECT_SOURCE_NONE = 0,
    APP_WIFIMAN_IMPL_STA_CONNECT_SOURCE_INITIAL,
    APP_WIFIMAN_IMPL_STA_CONNECT_SOURCE_RECONNECT,
} app_wifiman_impl_sta_connect_source_t;

typedef struct {
    dom_contracts_logger_leveled_t*       logger;
    dom_contracts_device_wifi_t*          wifi;
    dom_contracts_repository_wifi_t*      wifi_repository;
    dom_contracts_repository_preloaded_t* preloaded_repository;
    dom_contracts_network_interface_t*    network_interface;
    size_t                                reconnect_max_trials;
    bool                                  ap_auto_manage_enabled;
} app_wifiman_impl_cfg_t;

typedef struct {
    app_wifiman_impl_cfg_t                 cfg;
    bool                                   started;
    bool                                   ap_started;
    bool                                   event_callback_registered;
    bool                                   auto_reconnect_enabled;
    bool                                   sta_connection_commit_required;
    bool                                   ap_enabled_by_reconnect_threshold;
    app_wifiman_impl_sta_connect_source_t sta_connect_source;
    size_t                                 reconnect_trial_count;
} app_wifiman_impl_ctx_t;

#ifdef __cplusplus
}
#endif

#endif /* APPLICATION_WIFIMAN_IMPL_TYPES_H */
