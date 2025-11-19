#ifndef __CORE_SERVICE_H
#define __CORE_SERVICE_H

#include "esp_err.h"
#include "esp_http_server.h"
#include "haya/error.h"
#include "haya/log.h"
#include "haya/service/service.h"
#include "peripheral.h"

/**
 * @defgroup core_service_cfg Core Service Configuration
 * @brief Macros to enable and configure high-level system services.
 * @{
 */

/** @name HTTP Server Configuration */
///@{
#define SCG_HTTP_SERVER_ENABLE 1 ///< Set to 1 to enable the embedded HTTP server, 0 to disable.
#define SCG_HTTP_SERVER_PORT 80  ///< The TCP port to listen on (default is 80).
///@}

/** @name WiFi Manager Configuration */
///@{
#define SCG_WIFIMAN_ENABLE 1                       ///< Set to 1 to enable the WiFi Manager service (requires NVS and WiFi peripheral).
#define SCG_WIFIMAN_DEFAULT_AP_SSID "HayaOS-AP"    ///< The default SSID for the SoftAP when provisioning.
#define SCG_WIFIMAN_DEFAULT_AP_PASS "Haya12345678" ///< The default password for the SoftAP.

/* Dependency check: Wifiman requires NVS for storage and WiFi for networking */
#if SCG_WIFIMAN_ENABLE == 1 && (PCG_NVS_ENABLE != 1 || PCG_WIFI_ENABLE != 1)
#error "NVS and WiFi peripherals must be enabled to use Wifiman service"
#endif
///@}

/** @name MQTT Client Configuration */
///@{
#define SCG_MQTT_CLIENT_ENABLE 1 ///< Set to 1 to enable the MQTT client service.
///@}

/** @} */ // end of core_service_cfg group

/**
 * @brief A container for all initialized service handles.
 *
 * This struct is populated by coreServiceConfig() based on the
 * SCG_... macros defined in this file. It holds references to
 * services that utilize the underlying peripherals.
 */
typedef struct
{
#if SCG_HTTP_SERVER_ENABLE == 1
    httpd_handle_t server; /*!< @brief Handle for the HTTP server instance. */
#endif

#if SCG_WIFIMAN_ENABLE == 1
    Wifiman *wifiman; /*!< @brief Handle/Pointer to the WiFi Manager service instance. */
#endif

#if SCG_MQTT_CLIENT_ENABLE == 1
#endif
} CoreService;

/**
 * @brief Initializes high-level services and links them to peripherals.
 *
 * This function reads the configuration macros in this file and initializes
 * enabled services (HTTP Server, WiFi Manager, etc.). It requires a
 * populated CorePeripheral struct to access the necessary hardware resources.
 *
 * @param s A pointer to the CoreService struct to be filled with service handles.
 * @param p A pointer to an initialized CorePeripheral struct containing
 * hardware handles (NVS, WiFi, etc.) required by the services.
 */
void coreServiceConfig(CoreService *s, CorePeripheral *p);

#endif