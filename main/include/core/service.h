#ifndef __CORE_SERVICE_H
#define __CORE_SERVICE_H

#include "haya/error.h"
#include "haya/log.h"
#include "haya/service/service.h"
#include "peripheral.h"

#define SCG_WIFIMAN_ENABLE 1
#define SCG_WIFIMAN_DEFAULT_AP_SSID "HayaOS-AP"
#define SCG_WIFIMAN_DEFAULT_AP_PASS "Haya12345678"
#if SCG_WIFIMAN_ENABLE == 1 && (PCG_NVS_ENABLE != 1 || PCG_WIFI_ENABLE != 1)
#error "NVS and WiFi peripherals must be enabled to use Wifiman service"
#endif

#define SCG_MQFASTT_ENABLE 1

typedef struct
{
#if SCG_WIFIMAN_ENABLE == 1
    Wifiman *wifiman;
#endif
} CoreService;

void coreServiceConfig(CoreService *s, CorePeripheral *p);

#endif