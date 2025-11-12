#ifndef __HAYA_INTERFACE_GPIO_H
#define __HAYA_INTERFACE_GPIO_H

#include "esp_err.h"
#include "driver/gpio.h"

esp_err_t hyInterfaceGPIOSetup(
    uint64_t pin_mask,
    gpio_mode_t mode,
    gpio_pullup_t pullup_en,
    gpio_pulldown_t pulldown_en,
    gpio_int_type_t intr_type);

#endif