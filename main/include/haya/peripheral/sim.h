#ifndef __HAYA_PERIPHERAL_SIM_H
#define __HAYA_PERIPHERAL_SIM_H

#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_err.h"
#include "esp_netif.h"
#include "esp_netif_ppp.h"
#include "esp_modem_api.h"

esp_err_t hyPeripheralSIMSetup(
    esp_netif_t **netif,
    esp_modem_dce_t **dce,
    esp_modem_dce_device_t modem_device,
    const char *apn,
    const char *pin,
    uart_port_t uart_port,
    int pwrkey_pin,
    int tx_pin,
    int rx_pin,
    int baudrate);

#endif