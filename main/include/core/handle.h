#ifndef __CORE_HANDLE_H
#define __CORE_HANDLE_H

#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "haya/log.h"
#include "haya/app.h"
#include "interface.h"
#include "peripheral.h"

typedef struct
{
    CoreInterface *i;
} Core;

void coreSetup(Core *core);
void coreLoop(Core *core);
void coreRestart(uint32_t wait_ms);
void _coreFreeApp(HyAppHandle *app_handle);
void _coreRestartApp(HyAppHandle *app_handle);

#endif