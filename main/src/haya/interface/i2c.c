#include "haya/interface/i2c.h"

esp_err_t hyInterfaceI2CSetup(
    i2c_master_bus_handle_t *i2c_handle,
    i2c_port_t i2c_port,
    int sda_pin,
    int scl_pin)
{
    i2c_master_bus_config_t i2cbus_config = {
        .i2c_port = i2c_port,
        .sda_io_num = sda_pin,
        .scl_io_num = scl_pin,
        .clk_source = I2C_CLK_SRC_DEFAULT};

    return i2c_new_master_bus(
        &i2cbus_config,
        i2c_handle);
}