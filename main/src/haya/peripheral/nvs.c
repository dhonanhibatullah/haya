#include "haya/peripheral/nvs.h"

esp_err_t hyPeripheralNVSReset()
{
    return nvs_flash_erase();
}

esp_err_t hyPeripheralNVSSetup(
    nvs_handle_t *nvs_handle,
    const char *namespace,
    bool reset_on_fail)
{
    esp_err_t err = nvs_flash_init();
    if (reset_on_fail && (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND))
    {
        err = nvs_flash_erase();
        if (err != ESP_OK)
            return err;

        err = nvs_flash_init();
        if (err != ESP_OK)
            return err;
    }

    err = nvs_open(namespace, NVS_READWRITE, nvs_handle);
    if (err != ESP_OK)
    {
        nvs_flash_deinit();
        return err;
    }

    return ESP_OK;
}