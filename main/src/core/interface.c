#include "core/interface.h"

const char *CORE_INTERFACE_TAG = "core/interface";

void coreInterfaceConfig(CoreInterface *i)
{
    esp_err_t err;

#if ICG_SPI2_ENABLE == 1
    err = hyInterfaceSPISetup(
        SPI2_HOST,
        ICG_SPI2_MOSI_PIN,
        ICG_SPI2_MISO_PIN,
        ICG_SPI2_SCLK_PIN);
    if (err != ESP_OK)
    {
        hyLogError(
            CORE_INTERFACE_TAG,
            "failed to setup SPI2: %s, restarting device...",
            esp_err_to_name(err));
        esp_restart();
    }
    hyLogInfo(
        CORE_INTERFACE_TAG,
        "SPI2 setup success, MOSI: %d, MISO: %d, SCLK: %d",
        ICG_SPI2_MOSI_PIN,
        ICG_SPI2_MISO_PIN,
        ICG_SPI2_SCLK_PIN);
#endif

#if ICG_SPI3_ENABLE == 1
    err = hyInterfaceSPISetup(
        SPI3_HOST,
        ICG_SPI3_MOSI_PIN,
        ICG_SPI3_MISO_PIN,
        ICG_SPI3_SCLK_PIN);
    if (err != ESP_OK)
    {
        hyLogError(
            CORE_INTERFACE_TAG,
            "failed to setup SPI3: %s, restarting device...",
            esp_err_to_name(err));
        esp_restart();
    }
    hyLogInfo(
        CORE_INTERFACE_TAG,
        "SPI3 setup success, MOSI: %d, MISO: %d, SCLK: %d",
        ICG_SPI3_MOSI_PIN,
        ICG_SPI3_MISO_PIN,
        ICG_SPI3_SCLK_PIN);
#endif

#if ICG_I2C0_ENABLE == 1
    err = hyInterfaceI2CSetup(
        &i->i2c0_handle,
        I2C_NUM_0,
        ICG_I2C0_SDA_PIN,
        ICG_I2C0_SCL_PIN);
    if (err != ESP_OK)
    {
        hyLogError(
            CORE_INTERFACE_TAG,
            "failed to setup I2C0: %s, restarting device...",
            esp_err_to_name(err));
        esp_restart();
    }
    hyLogInfo(
        CORE_INTERFACE_TAG,
        "I2C0 setup success, SDA: %d, SCL: %d",
        ICG_I2C0_SDA_PIN,
        ICG_I2C0_SCL_PIN);
#endif

#if ICG_I2C1_ENABLE == 1
    err = hyInterfaceI2CSetup(
        &i->i2c1_handle,
        I2C_NUM_1,
        ICG_I2C1_SDA_PIN,
        ICG_I2C1_SCL_PIN);
    if (err != ESP_OK)
    {
        hyLogError(
            CORE_INTERFACE_TAG,
            "failed to setup I2C1: %s, restarting device...",
            esp_err_to_name(err));
        esp_restart();
    }
    hyLogInfo(
        CORE_INTERFACE_TAG,
        "I2C1 setup success, SDA: %d, SCL: %d",
        ICG_I2C1_SDA_PIN,
        ICG_I2C1_SCL_PIN);
#endif

#if ICG_UART1_ENABLE == 1
    err = hayaInterfaceUARTSetup(
        UART_NUM_1,
        ICG_UART1_TYPE,
        ICG_UART1_BAUDRATE,
        ICG_UART1_RX_PIN,
        ICG_UART1_TX_PIN,
        ICG_UART1_RTS_PIN,
        ICG_UART1_CTS_PIN,
        ICG_UART1_RX_BUF_SIZE,
        ICG_UART1_TX_BUF_SIZE,
        ICG_UART1_QUEUE_SIZE,
        &i->uart1_queue);
    if (err != ESP_OK)
    {
        hyLogError(
            CORE_INTERFACE_TAG,
            "failed to setup UART1: %s, restarting device...",
            esp_err_to_name(err));
        esp_restart();
    }
    hyLogInfo(
        CORE_INTERFACE_TAG,
        "UART1 setup success, BAUD: %d, RX: %d, TX: %d, RTS: %d, CTS: %d",
        ICG_UART1_BAUDRATE,
        ICG_UART1_RX_PIN,
        ICG_UART1_TX_PIN,
        ICG_UART1_RTS_PIN,
        ICG_UART1_CTS_PIN);
#endif

#if ICG_UART2_ENABLE == 1
    err = hayaInterfaceUARTSetup(
        UART_NUM_2,
        ICG_UART2_TYPE,
        ICG_UART2_BAUDRATE,
        ICG_UART2_RX_PIN,
        ICG_UART2_TX_PIN,
        ICG_UART2_RTS_PIN,
        ICG_UART2_CTS_PIN,
        ICG_UART2_RX_BUF_SIZE,
        ICG_UART2_TX_BUF_SIZE,
        ICG_UART2_QUEUE_SIZE,
        &i->uart2_queue);
    if (err != ESP_OK)
    {
        hyLogError(
            CORE_INTERFACE_TAG,
            "failed to setup UART2: %s, restarting device...",
            esp_err_to_name(err));
        esp_restart();
    }
    hyLogInfo(
        CORE_INTERFACE_TAG,
        "UART2 setup success, BAUD: %d, RX: %d, TX: %d, RTS: %d, CTS: %d",
        ICG_UART2_BAUDRATE,
        ICG_UART2_RX_PIN,
        ICG_UART2_TX_PIN,
        ICG_UART2_RTS_PIN,
        ICG_UART2_CTS_PIN);
#endif

#if ICG_TWAI_ENABLE == 1
#endif
}