#ifndef PRESENTATION_TASK_WIFIMAN_STA_RECONNECT_TYPES_H
#define PRESENTATION_TASK_WIFIMAN_STA_RECONNECT_TYPES_H

#include <stdbool.h>
#include <stdint.h>

#include "domain/usecases/wifiman.h"
#include "freertos/FreeRTOS.h"  // IWYU pragma: keep
#include "freertos/task.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PRES_TASK_WIFIMAN_STA_RECONNECT_DEFAULT_TASK_NAME   "wifiman_sta_reconnect"
#define PRES_TASK_WIFIMAN_STA_RECONNECT_DEFAULT_STACK_SIZE  4096
#define PRES_TASK_WIFIMAN_STA_RECONNECT_DEFAULT_PRIORITY    5
#define PRES_TASK_WIFIMAN_STA_RECONNECT_DEFAULT_INTERVAL_MS 5000

typedef struct {
    dom_usecases_wifiman_t* wifiman;
    const char*             task_name;
    uint32_t                stack_size;
    UBaseType_t             priority;
    uint32_t                interval_ms;
} pres_task_wifiman_sta_reconnect_cfg_t;

typedef struct pres_task_wifiman_sta_reconnect_t {
    pres_task_wifiman_sta_reconnect_cfg_t cfg;
    TaskHandle_t                          task_handle;
    bool                                  started;
    volatile bool                         stop_requested;
} pres_task_wifiman_sta_reconnect_t;

#ifdef __cplusplus
}
#endif

#endif /* PRESENTATION_TASK_WIFIMAN_STA_RECONNECT_TYPES_H */
