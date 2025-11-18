#ifndef __CORE_SERVICE_H
#define __CORE_SERVICE_H

#include "haya/error.h"
#include "haya/log.h"
#include "haya/service/service.h"
#include "peripheral.h"

#define PCG_WIFIMAN_ENABLE 1
#define PCG_WIFIMAN_DEFAULT_AP_SSID "HayaOS-AP"
#define PCG_WIFIMAN_DEFAULT_AP_PASS "Haya12345678"
#if PCG_WIFIMAN_ENABLE == 1 && (PCG_NVS_ENABLE != 1 || PCG_WIFI_ENABLE != 1)
#error "NVS and WiFi peripherals must be enabled to use Wifiman service"
#endif

typedef struct
{
#if PCG_WIFIMAN_ENABLE == 1
    Wifiman *wifiman;
#endif
} CoreService;

void coreServiceConfig(CoreService *s, CorePeripheral *p);

#endif