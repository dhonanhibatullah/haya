#include "haya/peripheral/sim.h"

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
    int baudrate)
{
    if (*netif == NULL || *dce == NULL || *apn == NULL)
        return ESP_ERR_INVALID_ARG;

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << pwrkey_pin),
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t err = gpio_config(&io_conf);
    if (err != ESP_OK)
        return err;

    gpio_set_level(pwrkey_pin, 0);
    vTaskDelay(pdMS_TO_TICKS(1200));
    gpio_set_level(pwrkey_pin, 1);
    vTaskDelay(pdMS_TO_TICKS(5000));

    esp_netif_config_t netif_cfg = ESP_NETIF_DEFAULT_PPP();
    *netif = esp_netif_new(&netif_cfg);
    if (*netif == NULL)
        return ESP_FAIL;

    esp_modem_dte_config_t dte_cfg = ESP_MODEM_DTE_DEFAULT_CONFIG();
    dte_cfg.uart_config.port_num = uart_port;
    dte_cfg.uart_config.tx_io_num = tx_pin;
    dte_cfg.uart_config.rx_io_num = rx_pin;
    dte_cfg.uart_config.baud_rate = baudrate;
    dte_cfg.uart_config.flow_control = ESP_MODEM_FLOW_CONTROL_NONE;

    esp_modem_dce_config_t dce_cfg = ESP_MODEM_DCE_DEFAULT_CONFIG(apn);
    *dce = esp_modem_new_dev(modem_device, &dte_cfg, &dce_cfg, *netif);
    if (*dce == NULL)
    {
        esp_netif_destroy(*netif);
        *netif = NULL;
        return ESP_FAIL;
    }

    uint8_t sync_trial = 0;
    while (1)
    {
        err = esp_modem_sync(*dce);
        if (err == ESP_OK)
            break;

        sync_trial += 1;
        if (sync_trial == 5)
        {
            esp_netif_destroy(*netif);
            *netif = NULL;
            return ESP_FAIL;
        }
    }

    err = esp_modem_set_mode(*dce, ESP_MODEM_MODE_DETECT);
    if (err != ESP_OK)
    {
        esp_netif_destroy(*netif);
        *netif = NULL;
        return err;
    }
    esp_modem_dce_mode_t mode = esp_modem_get_mode(*dce);
    if (mode != ESP_MODEM_MODE_COMMAND)
    {
        err = esp_modem_set_mode(*dce, ESP_MODEM_MODE_COMMAND);
        if (err != ESP_OK)
        {
            esp_netif_destroy(*netif);
            *netif = NULL;
            return err;
        }
    }

    if (pin != NULL)
    {
        bool pin_ok = false;
        if (esp_modem_read_pin(*dce, &pin_ok) == ESP_OK && pin_ok == false)
        {
            err = esp_modem_set_pin(*dce, pin);
            if (err != ESP_OK)
            {
                esp_netif_destroy(*netif);
                *netif = NULL;
                return err;
            }
        }
    }

    return ESP_OK;
}