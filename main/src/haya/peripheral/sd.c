#include "haya/peripheral/sd.h"

esp_err_t hyPeripheralSDSetup(
    sdmmc_card_t **card,
    spi_host_device_t spi_host,
    int cs_pin,
    const char *base_path,
    int max_files,
    size_t alloc_unit_size,
    bool format_on_fail)
{
    esp_vfs_fat_sdmmc_mount_config_t mount_cfg = {
        .format_if_mount_failed = format_on_fail,
        .max_files = max_files,
        .allocation_unit_size = alloc_unit_size};

    sdspi_device_config_t slot_cfg = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_cfg.gpio_cs = cs_pin;
    slot_cfg.host_id = spi_host;

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = spi_host;

    return esp_vfs_fat_sdspi_mount(
        base_path,
        &host,
        &slot_cfg,
        &mount_cfg,
        card);
}

HyErr hyPeripheralSDLogEnable(
    const char *dir_path,
    uint32_t file_keep_num,
    TickType_t rotation_period,
    TickType_t recover_period)
{
    return hyLogSaveEnable(
        dir_path,
        file_keep_num,
        rotation_period,
        recover_period);
}