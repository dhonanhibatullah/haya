#include "haya/interface/uart.h"

esp_err_t hayaInterfaceUARTSetup(
    uart_port_t uart_port,
    HyInterfaceUARTType uart_type,
    int baud_rate,
    int rx_pin,
    int tx_pin,
    int rts_pin,
    int cts_pin,
    int rx_buf_size,
    int tx_buf_size,
    int queue_size,
    QueueHandle_t *queue)
{
    if ((queue_size > 0) && (queue != NULL))
        return ESP_ERR_INVALID_ARG;

    uart_config_t uart_config = {
        .baud_rate = baud_rate,
        .source_clk = UART_SCLK_DEFAULT,
    };

    uint8_t flow_bits = (uart_type >> 6) & 0b11;
    switch (flow_bits)
    {
    case 0b00:
        uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
        break;
    case 0b01:
        uart_config.flow_ctrl = UART_HW_FLOWCTRL_CTS;
        break;
    case 0b10:
        uart_config.flow_ctrl = UART_HW_FLOWCTRL_RTS;
        break;
    case 0b11:
        uart_config.flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS;
        break;
    }

    uint8_t data_bits = (uart_type >> 4) & 0b11;
    switch (data_bits)
    {
    case 0b00:
        uart_config.data_bits = UART_DATA_5_BITS;
        break;
    case 0b01:
        uart_config.data_bits = UART_DATA_6_BITS;
        break;
    case 0b10:
        uart_config.data_bits = UART_DATA_7_BITS;
        break;
    case 0b11:
        uart_config.data_bits = UART_DATA_8_BITS;
        break;
    }

    uint8_t parity_bits = (uart_type >> 2) & 0b11;
    switch (parity_bits)
    {
    case 0b00:
        uart_config.parity = UART_PARITY_DISABLE;
        break;
    case 0b01:
        uart_config.parity = UART_PARITY_EVEN;
        break;
    case 0b10:
        uart_config.parity = UART_PARITY_ODD;
        break;
    }

    uint8_t stop_bits = uart_type & 0b11;
    switch (stop_bits)
    {
    case 0b00:
        uart_config.stop_bits = UART_STOP_BITS_1;
        break;
    case 0b01:
        uart_config.stop_bits = UART_STOP_BITS_1_5;
        break;
    case 0b10:
        uart_config.stop_bits = UART_STOP_BITS_2;
        break;
    }

    esp_err_t err;

    if (queue_size == 0)
    {
        err = uart_driver_install(uart_port, rx_buf_size, tx_buf_size, 0, NULL, 0);
        if (err != ESP_OK)
            return err;
    }
    else
    {
        err = uart_driver_install(uart_port, rx_buf_size, tx_buf_size, queue_size, queue, 0);
        if (err != ESP_OK)
            return err;
    }

    err = uart_param_config(uart_port, &uart_config);
    if (err != ESP_OK)
    {
        uart_driver_delete(uart_port);
        return err;
    }

    err = uart_set_pin(
        uart_port,
        tx_pin,
        rx_pin,
        rts_pin,
        cts_pin);
    if (err != ESP_OK)
    {
        uart_driver_delete(uart_port);
        return err;
    }

    return ESP_OK;
}