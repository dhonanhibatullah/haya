#ifndef __WIFIMAN_EVENT_HANDLE_H
#define __WIFIMAN_EVENT_HANDLE_H

#include "esp_err.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "haya/log.h"
#include "types.h"
#include "helper.h"

esp_err_t _hyWifimanEventRegister(HyWifiman *app);

esp_err_t _hyWifimanEventUnregister(HyWifiman *app);

void _hyWifimanEventHandler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data);

void _hyWifimanFallback(HyWifiman *app);

#endif