#ifndef __HAYA_PERIPHERAL_SD_H
#define __HAYA_PERIPHERAL_SD_H

#include "esp_err.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "haya/log.h"

esp_err_t hyPeripheralSDSetup(
    sdmmc_card_t **card,
    spi_host_device_t spi_host,
    int cs_pin,
    const char *base_path,
    int max_files,
    size_t alloc_unit_size,
    bool format_on_fail);

HyErr hyPeripheralSDLogEnable(
    const char *dir_path,
    uint32_t file_keep_num,
    TickType_t rotation_period,
    TickType_t recover_period);

#endif