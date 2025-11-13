#include "haya/peripheral/lfs.h"

esp_err_t hyPeripheralLFSSetup(
    const char *base_path,
    const char *partition_label,
    bool format_on_fail)
{
    esp_vfs_littlefs_conf_t conf = {
        .base_path = base_path,
        .partition_label = partition_label,
        .format_if_mount_failed = format_on_fail,
        .dont_mount = false};

    return esp_vfs_littlefs_register(&conf);
}