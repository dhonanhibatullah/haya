#ifndef __CORE_PERIPHERAL_H
#define __CORE_PERIPHERAL_H

#include "esp_system.h"
#include "haya/log.h"
#include "haya/peripheral/peripheral.h"

/**
 * @defgroup core_peripheral_cfg Core Peripheral Configuration
 * @brief Macros to enable and configure system peripherals.
 * @{
 */

/** @name NVS (Non-Volatile Storage) Configuration */
///@{
#define PCG_NVS_ENABLE 1           ///< Set to 1 to enable NVS, 0 to disable.
#define PCG_NVS_NAMESPACE "/nvs"   ///< The default namespace to use for NVS operations.
#define PCG_NVS_RESET_ON_FAIL true ///< Set to true to erase and re-init NVS if initialization fails.
#define PCG_NVS_RESET 0            ///< Set to 1 to force-erase NVS on boot (for development).
///@}

/** @name LittleFS Configuration */
///@{
#define PCG_LFS_ENABLE 0                   ///< Set to 1 to enable LittleFS, 0 to disable.
#define PCG_LFS_BASE_PATH "/lfs"           ///< The mount point for the LittleFS filesystem.
#define PCG_LFS_PARTITION_LABEL "/storage" ///< The partition label in the partition table.
#define PCG_LFS_FORMAT_ON_FAIL true        ///< Set to true to format the partition if mounting fails.
///@}

/** @name SD Card (SPI) Configuration */
///@{
#define PCG_SD_ENABLE 0                  ///< Set to 1 to enable the SD card, 0 to disable.
#define PCG_SD_SPI_HOST SPI3_HOST        ///< The SPI host to use (e.g., SPI2_HOST or SPI3_HOST).
#define PCG_SD_CS_PIN 13                 ///< GPIO pin for SD card Chip Select (CS).
#define PCG_SD_BASE_PATH "/sd"           ///< The mount point for the SD card filesystem.
#define PCG_SD_MAX_FILES 5               ///< Maximum number of open files.
#define PCG_SD_ALLOC_UNIT_SIZE 16 * 1024 ///< SD card allocation unit size.
#define PCG_SD_FORMAT_ON_FAIL true       ///< Set to true to format the card if mounting fails.
///@}

/** @name SD Card Logger Configuration */
///@{
#define PCG_SD_LOG_SAVE_ENABLE 0                                           ///< Set to 1 to enable saving `haya_log` output to the SD card.
#define PCG_SD_LOG_SAVE_DIR_PATH PCG_SD_BASE_PATH "/logs"                  ///< The directory on the SD card to save log files.
#define PCG_SD_LOG_SAVE_FILE_KEEP_NUM 24 * 30                              ///< The number of log files to keep in rotation.
#define PCG_SD_LOG_SAVE_FILE_ROTATION_PERIOD pdMS_TO_TICKS(1000 * 60 * 60) ///< The period, in ticks, to rotate to a new log file.
#define PCG_SD_LOG_SAVE_RECOVER_PERIOD pdMS_TO_TICKS(12 * 1000)            ///< The period, in ticks, to wait before retrying a file write.
///@}

/** @name WiFi Configuration */
///@{
#define PCG_WIFI_ENABLE 1 ///< Set to 1 to enable WiFi, 0 to disable.
///@}

/** @name Ethernet (W5500) Configuration */
///@{
#define PCG_ETH_ENABLE 0                          ///< Set to 1 to enable Ethernet, 0 to disable.
#define PCG_ETH_TYPE HY_PERIPHERAL_ETH_TYPE_W5500 ///< Ethernet module type.
#define PCG_ETH_SPI_HOST SPI2_HOST                ///< The SPI host to use for the Ethernet module.
#define PCG_ETH_SPI_CLOCK_SPEED 20 * 1000 * 1000  ///< SPI clock speed (e.g., 20MHz).
#define PCG_ETH_CS_PIN 5                          ///< GPIO pin for Ethernet module Chip Select (CS).
#define PCG_ETH_INT_PIN 10                        ///< GPIO pin for Ethernet module Interrupt.
#define PCG_ETH_RST_PIN 4                         ///< GPIO pin for Ethernet module Reset.
///@}

/** @name SIM Modem (SIM7000) Configuration */
///@{
#define PCG_SIM_ENABLE 0                                ///< Set to 1 to enable the SIM modem, 0 to disable.
#define PCG_SIM_MODEM_DEVICE_TYPE ESP_MODEM_DCE_SIM7000 ///< The modem model (from esp_modem).
#define PCG_SIM_APN "M2MAUTOTRONIC"                     ///< The Access Point Name (APN) for the SIM card.
#define PCG_SIM_PIN NULL                                ///< The SIM card PIN (or NULL if no PIN).
#define PCG_SIM_UART_PORT UART_NUM_2                    ///< The UART port connected to the modem.
#define PCG_SIM_BAUDRATE 115200                         ///< The baud rate for the UART connection.
#define PCG_SIM_TX_PIN GPIO_NUM_27                      ///< GPIO pin for UART TX.
#define PCG_SIM_RX_PIN GPIO_NUM_26                      ///< GPIO pin for UART RX.
#define PCG_SIM_PWRKEY_PIN GPIO_NUM_4                   ///< GPIO pin for toggling the modem's power key.
///@}

/** @} */ // end of core_peripheral_cfg group

/**
 * @brief A container for all initialized peripheral handles.
 *
 * This struct is populated by corePeripheralConfig() based on the
 * PCG_... macros defined in this file.
 */
typedef struct
{
#if PCG_NVS_ENABLE == 1
    nvs_handle_t nvs; /*!< @brief Handle for the NVS partition. */
#endif

#if PCG_SD_ENABLE == 1
    sdmmc_card_t *sd_card; /*!< @brief Handle for the mounted SD card. */
#endif

#if PCG_WIFI_ENABLE == 1
    esp_netif_t *wifi_ap_netif;  /*!< @brief Handle for the WiFi Access Point network interface. */
    esp_netif_t *wifi_sta_netif; /*!< @brief Handle for the WiFi Station network interface. */
#endif

#if PCG_ETH_ENABLE == 1
    esp_netif_t *eth_netif; /*!< @brief Handle for the Ethernet network interface. */
#endif

#if PCG_SIM_ENABLE == 1
    esp_netif_t *sim_netif;   /*!< @brief Handle for the SIM/PPP network interface. */
    esp_modem_dce_t *sim_dce; /*!< @brief Handle for the modem DCE (Data Communication Equipment). */
#endif
} CorePeripheral;

/**
 * @brief Initializes all hardware peripherals based on PCG_... macros.
 *
 * This function reads the configuration macros in this file and initializes
 * all enabled peripherals (NVS, filesystems, network interfaces, etc.).
 *
 * @param p A pointer to the CorePeripheral struct to be filled with
 * hardware handles.
 */
void corePeripheralConfig(CorePeripheral *p);

#endif