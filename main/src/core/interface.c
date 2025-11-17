#include "core/interface.h"

const char *CORE_INTERFACE_TAG = "core/interface";

void coreInterfaceConfig(CoreInterface *i)
{
    esp_err_t err;
    err = ESP_OK;

#if ICG_SPI2_ENABLE == 1
#ifndef INTERFACE_USED
#define INTERFACE_USED
#endif

    err = hyInterfaceSPISetup(
        SPI2_HOST,
        ICG_SPI2_MOSI_PIN,
        ICG_SPI2_MISO_PIN,
        ICG_SPI2_SCLK_PIN);
    if (err != ESP_OK)
    {
        hyLogError(
            CORE_INTERFACE_TAG,
            "failed to setup SPI2: %s",
            esp_err_to_name(err));
        goto restart_device;
    }
    hyLogInfo(
        CORE_INTERFACE_TAG,
        "SPI2 setup success, MOSI: %d, MISO: %d, SCLK: %d",
        ICG_SPI2_MOSI_PIN,
        ICG_SPI2_MISO_PIN,
        ICG_SPI2_SCLK_PIN);
#endif

#if ICG_SPI3_ENABLE == 1
#ifndef INTERFACE_USED
#define INTERFACE_USED
#endif

    err = hyInterfaceSPISetup(
        SPI3_HOST,
        ICG_SPI3_MOSI_PIN,
        ICG_SPI3_MISO_PIN,
        ICG_SPI3_SCLK_PIN);
    if (err != ESP_OK)
    {
        hyLogError(
            CORE_INTERFACE_TAG,
            "failed to setup SPI3: %s",
            esp_err_to_name(err));
        goto restart_device;
    }
    hyLogInfo(
        CORE_INTERFACE_TAG,
        "SPI3 setup success, MOSI: %d, MISO: %d, SCLK: %d",
        ICG_SPI3_MOSI_PIN,
        ICG_SPI3_MISO_PIN,
        ICG_SPI3_SCLK_PIN);
#endif

#if ICG_I2C0_ENABLE == 1
#ifndef INTERFACE_USED
#define INTERFACE_USED
#endif

    err = hyInterfaceI2CSetup(
        &i->i2c0_handle,
        I2C_NUM_0,
        ICG_I2C0_SDA_PIN,
        ICG_I2C0_SCL_PIN);
    if (err != ESP_OK)
    {
        hyLogError(
            CORE_INTERFACE_TAG,
            "failed to setup I2C0: %s",
            esp_err_to_name(err));
        goto restart_device;
    }
    hyLogInfo(
        CORE_INTERFACE_TAG,
        "I2C0 setup success, SDA: %d, SCL: %d",
        ICG_I2C0_SDA_PIN,
        ICG_I2C0_SCL_PIN);
#endif

#if ICG_I2C1_ENABLE == 1
#ifndef INTERFACE_USED
#define INTERFACE_USED
#endif

    err = hyInterfaceI2CSetup(
        &i->i2c1_handle,
        I2C_NUM_1,
        ICG_I2C1_SDA_PIN,
        ICG_I2C1_SCL_PIN);
    if (err != ESP_OK)
    {
        hyLogError(
            CORE_INTERFACE_TAG,
            "failed to setup I2C1: %s",
            esp_err_to_name(err));
        goto restart_device;
    }
    hyLogInfo(
        CORE_INTERFACE_TAG,
        "I2C1 setup success, SDA: %d, SCL: %d",
        ICG_I2C1_SDA_PIN,
        ICG_I2C1_SCL_PIN);
#endif

#if ICG_UART1_ENABLE == 1
#ifndef INTERFACE_USED
#define INTERFACE_USED
#endif

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
            "failed to setup UART1: %s",
            esp_err_to_name(err));
        goto restart_device;
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
#ifndef INTERFACE_USED
#define INTERFACE_USED
#endif

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
            "failed to setup UART2: %s",
            esp_err_to_name(err));
        goto restart_device;
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
#ifndef INTERFACE_USED
#define INTERFACE_USED
#endif

#if ICG_TWAI_ENABLE_FILTER == 1
    uint32_t filt_id_list[] = ICG_TWAI_ID_LIST;
#else
    uint32_t *filt_id_list = NULL;
#endif
    err = hyInterfaceTWAISetup(
        &i->twai_handle,
        ICG_TWAI_TX_PIN,
        ICG_TWAI_RX_PIN,
        ICG_TWAI_BITRATE,
        filt_id_list,
        ICG_TWAI_ID_COUNT,
        ICG_TWAI_MASK,
        ICG_TWAI_EXTENDED);
    if (err != ESP_OK)
    {
        hyLogError(
            CORE_INTERFACE_TAG,
            "failed to setup TWAI: %s",
            esp_err_to_name(err));
        goto restart_device;
    }
    hyLogInfo(
        CORE_INTERFACE_TAG,
        "TWAI setup success: RATE: %d, TX: %d, RX: %d",
        ICG_TWAI_BITRATE,
        ICG_TWAI_TX_PIN,
        ICG_TWAI_RX_PIN);
#endif

#if ICG_GPIO0_ENABLE == 1
#ifndef INTERFACE_USED
#define INTERFACE_USED
#endif

    err = hyInterfaceGPIOSetup(
        ICG_GPIO0_PIN_MASK,
        ICG_GPIO0_MODE,
        ICG_GPIO0_PULLUP,
        ICG_GPIO0_PULLDOWN,
        ICG_GPIO0_INTR_TYPE);
    if (err != ESP_OK)
    {
        hyLogError(
            CORE_INTERFACE_TAG,
            "failed to setup GPIO0: %s",
            esp_err_to_name(err));
        goto restart_device;
    }
    hyLogInfo(
        CORE_INTERFACE_TAG,
        "GPIO0 setup success");
#endif

#if ICG_GPIO1_ENABLE == 1
#ifndef INTERFACE_USED
#define INTERFACE_USED
#endif

    err = hyInterfaceGPIOSetup(
        ICG_GPIO1_PIN_MASK,
        ICG_GPIO1_MODE,
        ICG_GPIO1_PULLUP,
        ICG_GPIO1_PULLDOWN,
        ICG_GPIO1_INTR_TYPE);
    if (err != ESP_OK)
    {
        hyLogError(
            CORE_INTERFACE_TAG,
            "failed to setup GPIO1: %s",
            esp_err_to_name(err));
        goto restart_device;
    }
    hyLogInfo(
        CORE_INTERFACE_TAG,
        "GPIO1 setup success");
#endif

#if ICG_GPIO2_ENABLE == 1
#ifndef INTERFACE_USED
#define INTERFACE_USED
#endif

    err = hyInterfaceGPIOSetup(
        ICG_GPIO2_PIN_MASK,
        ICG_GPIO2_MODE,
        ICG_GPIO2_PULLUP,
        ICG_GPIO2_PULLDOWN,
        ICG_GPIO2_INTR_TYPE);
    if (err != ESP_OK)
    {
        hyLogError(
            CORE_INTERFACE_TAG,
            "failed to setup GPIO2: %s",
            esp_err_to_name(err));
        goto restart_device;
    }
    hyLogInfo(
        CORE_INTERFACE_TAG,
        "GPIO2 setup success");
#endif

#if ICG_GPIO3_ENABLE == 1
#ifndef INTERFACE_USED
#define INTERFACE_USED
#endif

    err = hyInterfaceGPIOSetup(
        ICG_GPIO3_PIN_MASK,
        ICG_GPIO3_MODE,
        ICG_GPIO3_PULLUP,
        ICG_GPIO3_PULLDOWN,
        ICG_GPIO3_INTR_TYPE);
    if (err != ESP_OK)
    {
        hyLogError(
            CORE_INTERFACE_TAG,
            "failed to setup GPIO3: %s",
            esp_err_to_name(err));
        goto restart_device;
    }
    hyLogInfo(
        CORE_INTERFACE_TAG,
        "GPIO3 setup success");
#endif

    return;

#ifdef INTERFACE_USED
restart_device:
    hyLogWarn(
        CORE_INTERFACE_TAG,
        "restarting device in 3s...");
    vTaskDelay(pdMS_TO_TICKS(3000));
    esp_restart();
#endif
}