#ifndef __CORE_SERVICE_H
#define __CORE_SERVICE_H

#include "esp_err.h"
#include "esp_http_server.h"
#include "haya/error.h"
#include "haya/log.h"
#include "haya/service/service.h"
#include "peripheral.h"

#define SCG_HTTP_SERVER_ENABLE 1
#define SCG_HTTP_SERVER_PORT 80

#define SCG_WIFIMAN_ENABLE 1
#define SCG_WIFIMAN_DEFAULT_AP_SSID "HayaOS-AP"
#define SCG_WIFIMAN_DEFAULT_AP_PASS "Haya12345678"
#if SCG_WIFIMAN_ENABLE == 1 && (PCG_NVS_ENABLE != 1 || PCG_WIFI_ENABLE != 1)
#error "NVS and WiFi peripherals must be enabled to use Wifiman service"
#endif

#define SCG_MQTT_CLIENT_ENABLE 1

typedef struct
{
#if SCG_HTTP_SERVER_ENABLE == 1
    httpd_handle_t server;
#endif

#if SCG_WIFIMAN_ENABLE == 1
    Wifiman *wifiman;
#endif

#if SCG_MQTT_CLIENT_ENABLE == 1
#endif
} CoreService;

void coreServiceConfig(CoreService *s, CorePeripheral *p);

#endif