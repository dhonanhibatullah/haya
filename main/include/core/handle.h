#ifndef __CORE_HANDLE_H
#define __CORE_HANDLE_H

#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "haya/log.h"
#include "haya/app.h"
#include "config_interface.h"
#include "config_peripheral.h"

void coreSetup();
void coreLoop();
void coreRestart(uint32_t wait_ms);
void _coreFreeApp(HyAppHandle *app_handle);
void _coreRestartApp(HyAppHandle *app_handle);

#endif