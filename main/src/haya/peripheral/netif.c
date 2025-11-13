#include "haya/peripheral/netif.h"

esp_err_t hyPeripheralNetifSetup()
{
    esp_err_t err = esp_event_loop_create_default();
    if (err != ESP_OK)
        return err;

    err = esp_netif_init();
    if (err != ESP_OK)
    {
        esp_event_loop_delete_default();
        return err;
    }

    return ESP_OK;
}