#include "haya/interface/gpio.h"

esp_err_t hyInterfaceGPIOSetup(
    uint64_t pin_mask,
    gpio_mode_t mode,
    gpio_pullup_t pullup_en,
    gpio_pulldown_t pulldown_en,
    gpio_int_type_t intr_type)
{
    gpio_config_t config = {
        .pin_bit_mask = pin_mask,
        .mode = mode,
        .pull_up_en = pullup_en,
        .pull_down_en = pulldown_en,
        .intr_type = intr_type};
    return gpio_config(&config);
}