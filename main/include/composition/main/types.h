#ifndef COMPOSITION_MAIN_TYPES_H
#define COMPOSITION_MAIN_TYPES_H

#include "domain/contracts/device/ethernet.h"
#include "domain/contracts/device/wifi.h"
#include "domain/contracts/logger/leveled.h"
#include "esp_http_server.h"
#include "nvs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    nvs_handle_t   nvs_handle;
    httpd_handle_t http_handle;
} cmp_main_driver_t;

typedef struct {
    dom_contracts_logger_leveled_t*  logger;
    dom_contracts_device_ethernet_t* ethernet;
    dom_contracts_device_wifi_t*     wifi;
} cmp_main_infrastructure_t;

typedef struct {
} cmp_main_application_t;

typedef struct {
} cmp_main_presentation_t;

typedef struct {
    cmp_main_driver_t*         driver;
    cmp_main_infrastructure_t* infrastructure;
    cmp_main_application_t*    application;
    cmp_main_presentation_t*   presentation;
} cmp_main_launcher_t;

#ifdef __cplusplus
}
#endif

#endif /* COMPOSITION_MAIN_TYPES_H */