#ifndef __HAYA_INTERFACE_I2C_H
#define __HAYA_INTERFACE_I2C_H

#include "esp_err.h"
#include "driver/i2c_master.h"

esp_err_t hyInterfaceI2CSetup(
    i2c_master_bus_handle_t *i2c_handle,
    i2c_port_t i2c_port,
    int sda_pin,
    int scl_pin);

#endif