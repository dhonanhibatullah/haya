#ifndef __WIFIMAN_HELPER_H
#define __WIFIMAN_HELPER_H

#include "esp_wifi.h"
#include "types.h"

esp_err_t _hyWifimanNVSInit(HyWifiman *app);

esp_err_t _hyWifimanNVSGetAPConfig(HyWifiman *app, wifi_config_t *wifi_cfg);

esp_err_t _hyWifimanNVSGetSTAConfig(HyWifiman *app, wifi_config_t *wifi_cfg);

esp_err_t _hyWifimanWiFiInit(HyWifiman *app);

esp_err_t _hyWifimanWiFiDeinit();

#endif