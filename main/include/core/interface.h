#ifndef __CORE_INTERFACE_H
#define __CORE_INTERFACE_H

#include "esp_system.h"
#include "haya/log.h"
#include "haya/interface/interface.h"

/**
 * @defgroup core_interface_cfg Core Interface Configuration
 * @brief Macros to enable and configure hardware peripherals.
 * @{
 */

/** @name SPI2 Configuration (HSPI) */
///@{
#define ICG_SPI2_ENABLE 0             ///< Set to 1 to enable SPI2 (HSPI), 0 to disable.
#define ICG_SPI2_MOSI_PIN GPIO_NUM_23 ///< GPIO pin for SPI2 MOSI.
#define ICG_SPI2_MISO_PIN GPIO_NUM_19 ///< GPIO pin for SPI2 MISO.
#define ICG_SPI2_SCLK_PIN GPIO_NUM_18 ///< GPIO pin for SPI2 SCLK.
///@}

/** @name SPI3 Configuration (VSPI) */
///@{
#define ICG_SPI3_ENABLE 0             ///< Set to 1 to enable SPI3 (VSPI), 0 to disable.
#define ICG_SPI3_MOSI_PIN GPIO_NUM_15 ///< GPIO pin for SPI3 MOSI.
#define ICG_SPI3_MISO_PIN GPIO_NUM_2  ///< GPIO pin for SPI3 MISO.
#define ICG_SPI3_SCLK_PIN GPIO_NUM_14 ///< GPIO pin for SPI3 SCLK.
///@}

/** @name I2C0 Configuration */
///@{
#define ICG_I2C0_ENABLE 0            ///< Set to 1 to enable I2C Port 0, 0 to disable.
#define ICG_I2C0_SDA_PIN GPIO_NUM_21 ///< GPIO pin for I2C0 SDA.
#define ICG_I2C0_SCL_PIN GPIO_NUM_22 ///< GPIO pin for I2C0 SCL.
///@}

/** @name I2C1 Configuration */
///@{
#define ICG_I2C1_ENABLE 0            ///< Set to 1 to enable I2C Port 1, 0 to disable.
#define ICG_I2C1_SDA_PIN GPIO_NUM_32 ///< GPIO pin for I2C1 SDA.
#define ICG_I2C1_SCL_PIN GPIO_NUM_33 ///< GPIO pin for I2C1 SCL.
///@}

/** @name UART1 Configuration */
///@{
#define ICG_UART1_ENABLE 0                               ///< Set to 1 to enable UART1, 0 to disable.
#define ICG_UART1_TYPE HY_INTERFACE_UART_TYPE_NOFLOW_8N1 ///< Hardware flow control setting for UART1.
#define ICG_UART1_BAUDRATE 9600                          ///< Baud rate for UART1.
#define ICG_UART1_RX_PIN GPIO_NUM_18                     ///< GPIO pin for UART1 RX.
#define ICG_UART1_TX_PIN GPIO_NUM_5                      ///< GPIO pin for UART1 TX.
#define ICG_UART1_RTS_PIN UART_PIN_NO_CHANGE             ///< GPIO pin for UART1 RTS (UART_PIN_NO_CHANGE if unused).
#define ICG_UART1_CTS_PIN UART_PIN_NO_CHANGE             ///< GPIO pin for UART1 CTS (UART_PIN_NO_CHANGE if unused).
#define ICG_UART1_RX_BUF_SIZE 1024                       ///< RX buffer size for UART1 driver.
#define ICG_UART1_TX_BUF_SIZE 0                          ///< TX buffer size for UART1 driver (0 = blocking writes).
#define ICG_UART1_QUEUE_SIZE 0                           ///< UART1 FreeRTOS queue size (0 = driver does not use a queue).
///@}

/** @name UART2 Configuration */
///@{
#define ICG_UART2_ENABLE 0                               ///< Set to 1 to enable UART2, 0 to disable.
#define ICG_UART2_TYPE HY_INTERFACE_UART_TYPE_NOFLOW_8N1 ///< Hardware flow control setting for UART2.
#define ICG_UART2_BAUDRATE 115200                        ///< Baud rate for UART2.
#define ICG_UART2_RX_PIN GPIO_NUM_26                     ///< GPIO pin for UART2 RX.
#define ICG_UART2_TX_PIN GPIO_NUM_27                     ///< GPIO pin for UART2 TX.
#define ICG_UART2_RTS_PIN UART_PIN_NO_CHANGE             ///< GPIO pin for UART2 RTS (UART_PIN_NO_CHANGE if unused).
#define ICG_UART2_CTS_PIN UART_PIN_NO_CHANGE             ///< GPIO pin for UART2 CTS (UART_PIN_NO_CHANGE if unused).
#define ICG_UART2_RX_BUF_SIZE 1024                       ///< RX buffer size for UART2 driver.
#define ICG_UART2_TX_BUF_SIZE 0                          ///< TX buffer size for UART2 driver (0 = blocking writes).
#define ICG_UART2_QUEUE_SIZE 16                          ///< UART2 FreeRTOS queue size (non-zero enables the event queue).
///@}

/** @name TWAI (CAN) Configuration */
///@{
#define ICG_TWAI_ENABLE 0               ///< Set to 1 to enable CAN, 0 to disable.
#define ICG_TWAI_TX_PIN GPIO_NUM_5      ///< GPIO pin for CAN TX.
#define ICG_TWAI_RX_PIN GPIO_NUM_4      ///< GPIO pin for CAN RX.
#define ICG_TWAI_BITRATE 500000         ///< Bitrate in bits/s (e.g., 500000 for 500 kbit/s).
#define ICG_TWAI_ENABLE_FILTER 1        ///< Set to 1 to enable CAN message filtering.
#define ICG_TWAI_ID_LIST {0x123, 0x456} ///< List of CAN IDs to accept when filtering is enabled.
#define ICG_TWAI_ID_COUNT 2             ///< Number of CAN IDs in the ID list.
#define ICG_TWAI_MASK 0x7FF             ///< CAN acceptance filter mask. Bits set to 1 are "don't care".
#define ICG_TWAI_EXTENDED 1             ///< Set to 1 for extended (29-bit) CAN IDs, 0 for standard (11-bit).
///@}

/** @name GPIO Configuration 0 */
///@{
#define ICG_GPIO0_ENABLE 0                                         ///< Set to 1 to enable GPIO Group 0, 0 to disable.
#define ICG_GPIO0_PIN_MASK (1 << GPIO_NUM_33) | (1 << GPIO_NUM_32) ///< Bitmask of pins to configure for Group 0.
#define ICG_GPIO0_MODE GPIO_MODE_OUTPUT                            ///< GPIO mode (e.g., GPIO_MODE_OUTPUT, GPIO_MODE_INPUT).
#define ICG_GPIO0_PULLUP GPIO_PULLUP_DISABLE                       ///< Enable pull-up resistor (GPIO_PULLUP_ENABLE or GPIO_PULLUP_DISABLE).
#define ICG_GPIO0_PULLDOWN GPIO_PULLDOWN_DISABLE                   ///< Enable pull-down resistor (GPIO_PULLDOWN_ENABLE or GPIO_PULLDOWN_DISABLE).
#define ICG_GPIO0_INTR_TYPE GPIO_INTR_DISABLE                      ///< Interrupt type for this pin group (e.g., GPIO_INTR_NEGEDGE).
///@}

/** @name GPIO Configuration 1 */
///@{
#define ICG_GPIO1_ENABLE 0                                         ///< Set to 1 to enable GPIO Group 1, 0 to disable.
#define ICG_GPIO1_PIN_MASK (1 << GPIO_NUM_33) | (1 << GPIO_NUM_32) ///< Bitmask of pins to configure for Group 1.
#define ICG_GPIO1_MODE GPIO_MODE_OUTPUT                            ///< GPIO mode (e.g., GPIO_MODE_OUTPUT, GPIO_MODE_INPUT).
#define ICG_GPIO1_PULLUP GPIO_PULLUP_DISABLE                       ///< Enable pull-up resistor (GPIO_PULLUP_ENABLE or GPIO_PULLUP_DISABLE).
#define ICG_GPIO1_PULLDOWN GPIO_PULLDOWN_DISABLE                   ///< Enable pull-down resistor (GPIO_PULLDOWN_ENABLE or GPIO_PULLDOWN_DISABLE).
#define ICG_GPIO1_INTR_TYPE GPIO_INTR_DISABLE                      ///< Interrupt type for this pin group (e.g., GPIO_INTR_NEGEDGE).
///@}

/** @name GPIO Configuration 2 */
///@{
#define ICG_GPIO2_ENABLE 0                                         ///< Set to 1 to enable GPIO Group 2, 0 to disable.
#define ICG_GPIO2_PIN_MASK (1 << GPIO_NUM_33) | (1 << GPIO_NUM_32) ///< Bitmask of pins to configure for Group 2.
#define ICG_GPIO2_MODE GPIO_MODE_OUTPUT                            ///< GPIO mode (e.g., GPIO_MODE_OUTPUT, GPIO_MODE_INPUT).
#define ICG_GPIO2_PULLUP GPIO_PULLUP_DISABLE                       ///< Enable pull-up resistor (GPIO_PULLUP_ENABLE or GPIO_PULLUP_DISABLE).
#define ICG_GPIO2_PULLDOWN GPIO_PULLDOWN_DISABLE                   ///< Enable pull-down resistor (GPIO_PULLDOWN_ENABLE or GPIO_PULLDOWN_DISABLE).
#define ICG_GPIO2_INTR_TYPE GPIO_INTR_DISABLE                      ///< Interrupt type for this pin group (e.g., GPIO_INTR_NEGEDGE).
///@}

/** @name GPIO Configuration 3 */
///@{
#define ICG_GPIO3_ENABLE 0                                         ///< Set to 1 to enable GPIO Group 3, 0 to disable.
#define ICG_GPIO3_PIN_MASK (1 << GPIO_NUM_33) | (1 << GPIO_NUM_32) ///< Bitmask of pins to configure for Group 3.
#define ICG_GPIO3_MODE GPIO_MODE_OUTPUT                            ///< GPIO mode (e.g., GPIO_MODE_OUTPUT, GPIO_MODE_INPUT).
#define ICG_GPIO3_PULLUP GPIO_PULLUP_DISABLE                       ///< Enable pull-up resistor (GPIO_PULLUP_ENABLE or GPIO_PULLUP_DISABLE).
#define ICG_GPIO3_PULLDOWN GPIO_PULLDOWN_DISABLE                   ///< Enable pull-down resistor (GPIO_PULLDOWN_ENABLE or GPIO_PULLDOWN_DISABLE).
#define ICG_GPIO3_INTR_TYPE GPIO_INTR_DISABLE                      ///< Interrupt type for this pin group (e.g., GPIO_INTR_NEGEDGE).
///@}

/** @} */ // end of core_interface_cfg group

/**
 * @brief A container for all initialized hardware interface handles.
 *
 * This struct is populated by coreInterfaceConfig() based on the
 * ICG_... macros defined in this file.
 */
typedef struct
{
#if ICG_I2C0_ENABLE == 1
    i2c_master_bus_handle_t i2c0_handle; /*!< @brief Handler for I2C0 bus. */
#endif

#if ICG_I2C1_ENABLE == 1
    i2c_master_bus_handle_t i2c1_handle; /*!< @brief Handler for I2C1 bus. */
#endif

#if ICG_UART1_ENABLE == 1
    QueueHandle_t uart1_queue; /*!< @brief Queue handle for UART 1 events. */
#endif

#if ICG_UART2_ENABLE == 1
    QueueHandle_t uart2_queue; /*!< @brief Queue handle for UART 2 events. */
#endif

#if ICG_TWAI_ENABLE == 1
    twai_node_handle_t twai_handle; /*!< @brief Handler for TWAI/CAN interface. */
#endif
} CoreInterface;

/**
 * @brief Initializes all hardware interfaces based on ICG_... macros.
 *
 * This function reads the configuration macros in this file and initializes
 * all enabled peripherals (SPI, I2C, UART, TWAI, GPIO).
 *
 * @param i A pointer to the CoreInterface struct to be filled with
 * hardware handles.
 */
void coreInterfaceConfig(CoreInterface *i);

#endif