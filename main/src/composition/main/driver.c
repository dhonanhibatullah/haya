#include "composition/main/driver.h"  // IWYU pragma: keep

#include "composition/main/config.h"     // IWYU pragma: keep
#include "domain/models/error.h"         // IWYU pragma: keep
#include "driver/gpio.h"                 // IWYU pragma: keep
#include "driver/i2c_master.h"           // IWYU pragma: keep
#include "driver/sdspi_host.h"           // IWYU pragma: keep
#include "driver/spi_common.h"           // IWYU pragma: keep
#include "driver/spi_master.h"           // IWYU pragma: keep
#include "esp_err.h"                     // IWYU pragma: keep
#include "esp_eth_driver.h"              // IWYU pragma: keep
#include "esp_eth_mac.h"                 // IWYU pragma: keep
#include "esp_eth_mac_w5500.h"           // IWYU pragma: keep
#include "esp_eth_phy.h"                 // IWYU pragma: keep
#include "esp_eth_phy_w5500.h"           // IWYU pragma: keep
#include "esp_event.h"                   // IWYU pragma: keep
#include "esp_littlefs.h"                // IWYU pragma: keep
#include "esp_log.h"                     // IWYU pragma: keep
#include "esp_netif.h"                   // IWYU pragma: keep
#include "esp_vfs_fat.h"                 // IWYU pragma: keep
#include "esp_wifi.h"                    // IWYU pragma: keep
#include "hal/i2c_types.h"               // IWYU pragma: keep
#include "hal/spi_types.h"               // IWYU pragma: keep
#include "nimble/nimble_port.h"          // IWYU pragma: keep
#include "nvs.h"                         // IWYU pragma: keep
#include "nvs_flash.h"                   // IWYU pragma: keep
#include "sdmmc_cmd.h"                   // IWYU pragma: keep
#include "services/gap/ble_svc_gap.h"    // IWYU pragma: keep
#include "services/gatt/ble_svc_gatt.h"  // IWYU pragma: keep
#include "soc/clk_tree_defs.h"           // IWYU pragma: keep

#define TAG_PATH "main"

/* Init Flags for Deinitizalization Sequence */

static bool init_isr      = false;
static bool init_i2c_0    = false;
static bool init_i2c_1    = false;
static bool init_spi_2    = false;
static bool init_spi_3    = false;
static bool init_nvs      = false;
static bool init_littlefs = false;
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_SD_CARD_ENABLE
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_SD_CARD_USE_SDSPI
static bool init_sd_card_sdspi = false;
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_SD_CARD_USE_SDSPI */
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_SD_CARD_ENABLE */
static bool init_event_loop         = false;
static bool init_netif              = false;
static bool init_wifi               = false;
static bool init_ethernet_w5500_mac = false;
static bool init_ethernet_w5500_phy = false;
static bool init_ethernet_w5500     = false;
static bool init_ble                = false;
static bool init_http_server        = false;
static bool init_mqtt_client        = false;

dom_models_error_t cmp_main_driver_init(cmp_main_launcher_t* launcher) {
    const char* tag = TAG_PATH "/init";
    esp_err_t   err;

    /* ISR */

#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_ISR_ENABLE

    err = gpio_install_isr_service(cmp_main_config.driver.isr_intr_alloc_flag);
    if (err != ESP_OK) {
        ESP_LOGE(tag, "Failed to install ISR: %s", esp_err_to_name(err));
        return DOMAIN_MODELS_ERROR_FAILURE;
    }

    init_isr = true;
    ESP_LOGI(tag, "ISR installed");

#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_ISR_ENABLE */

    /* I2C 0 */

#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_I2C_0_ENABLE

    i2c_master_bus_config_t i2c_0_bus_cfg = {
        .i2c_port                     = I2C_NUM_0,
        .clk_source                   = I2C_CLK_SRC_DEFAULT,
        .sda_io_num                   = cmp_main_config.driver.i2c_0_sda_pin,
        .scl_io_num                   = cmp_main_config.driver.i2c_0_scl_pin,
        .glitch_ignore_cnt            = 7,
        .flags.enable_internal_pullup = cmp_main_config.driver.i2c_0_enable_internal_pullup,
    };
    err = i2c_new_master_bus(&i2c_0_bus_cfg, &launcher->driver.i2c_0_bus_handle);
    if (err != ESP_OK) {
        ESP_LOGE(tag, "Failed to create I2C 0 Bus: %s", esp_err_to_name(err));
        cmp_main_driver_deinit(launcher);
        return DOMAIN_MODELS_ERROR_FAILURE;
    }

    init_i2c_0 = true;
    ESP_LOGI(tag, "I2C 0 bus initialized");

#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_I2C_0_ENABLE */

    /* I2C 1 */

#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_I2C_1_ENABLE

    i2c_master_bus_config_t i2c_1_bus_cfg = {
        .i2c_port                     = I2C_NUM_1,
        .clk_source                   = I2C_CLK_SRC_DEFAULT,
        .sda_io_num                   = cmp_main_config.driver.i2c_1_sda_pin,
        .scl_io_num                   = cmp_main_config.driver.i2c_1_scl_pin,
        .glitch_ignore_cnt            = 7,
        .flags.enable_internal_pullup = cmp_main_config.driver.i2c_1_enable_internal_pullup,
    };
    err = i2c_new_master_bus(&i2c_1_bus_cfg, &launcher->driver.i2c_1_bus_handle);
    if (err != ESP_OK) {
        ESP_LOGE(tag, "Failed to create I2C 1 Bus: %s", esp_err_to_name(err));
        cmp_main_driver_deinit(launcher);
        return DOMAIN_MODELS_ERROR_FAILURE;
    }

    init_i2c_1 = true;
    ESP_LOGI(tag, "I2C 1 bus initialized");

#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_I2C_1_ENABLE */

    /* SPI 2 */

#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_SPI_2_ENABLE

    spi_bus_config_t spi_2_bus_cfg = {
        .miso_io_num     = cmp_main_config.driver.spi_2_miso_pin,
        .mosi_io_num     = cmp_main_config.driver.spi_2_mosi_pin,
        .sclk_io_num     = cmp_main_config.driver.spi_2_sclk_pin,
        .quadwp_io_num   = cmp_main_config.driver.spi_2_quadwp_pin,
        .quadhd_io_num   = cmp_main_config.driver.spi_2_quadhd_pin,
        .max_transfer_sz = cmp_main_config.driver.spi_2_max_transfer_size,
    };
    err = spi_bus_initialize(SPI2_HOST, &spi_2_bus_cfg, SPI_DMA_CH_AUTO);
    if (err != ESP_OK) {
        ESP_LOGE(tag, "Failed to create SPI 2 (HSPI) bus: %s", esp_err_to_name(err));
        cmp_main_driver_deinit(launcher);
        return DOMAIN_MODELS_ERROR_FAILURE;
    }

    init_spi_2 = true;
    ESP_LOGI(tag, "SPI 2 (HSPI) bus initialized");

#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_SPI_2_ENABLE */

    /* SPI 3 */

#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_SPI_3_ENABLE

    spi_bus_config_t spi_3_bus_cfg = {
        .miso_io_num     = cmp_main_config.driver.spi_3_miso_pin,
        .mosi_io_num     = cmp_main_config.driver.spi_3_mosi_pin,
        .sclk_io_num     = cmp_main_config.driver.spi_3_sclk_pin,
        .quadwp_io_num   = cmp_main_config.driver.spi_3_quadwp_pin,
        .quadhd_io_num   = cmp_main_config.driver.spi_3_quadhd_pin,
        .max_transfer_sz = cmp_main_config.driver.spi_3_max_transfer_size,
    };
    err = spi_bus_initialize(SPI3_HOST, &spi_3_bus_cfg, SPI_DMA_CH_AUTO);
    if (err != ESP_OK) {
        ESP_LOGE(tag, "Failed to create SPI 3 (VSPI) bus: %s", esp_err_to_name(err));
        cmp_main_driver_deinit(launcher);
        return DOMAIN_MODELS_ERROR_FAILURE;
    }

    init_spi_3 = true;
    ESP_LOGI(tag, "SPI 3 (VSPI) bus initialized");

#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_SPI_3_ENABLE */

    /* NVS */

#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_NVS_ENABLE

    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(tag, "NVS requires erase: %s", esp_err_to_name(err));
        err = nvs_flash_erase();
        if (err != ESP_OK) {
            ESP_LOGE(tag, "Failed to erase NVS partition: %s", esp_err_to_name(err));
            cmp_main_driver_deinit(launcher);
            return DOMAIN_MODELS_ERROR_FAILURE;
        }
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        ESP_LOGE(tag, "Failed to initialize NVS: %s", esp_err_to_name(err));
        cmp_main_driver_deinit(launcher);
        return DOMAIN_MODELS_ERROR_FAILURE;
    }

    err = nvs_open(
        cmp_main_config.driver.nvs_namespace,
        NVS_READWRITE,
        &launcher->driver.nvs_handle
    );
    if (err != ESP_OK) {
        ESP_LOGE(tag, "Failed to open NVS '%s': %s", cmp_main_config.driver.nvs_namespace, esp_err_to_name(err));
        cmp_main_driver_deinit(launcher);
        return DOMAIN_MODELS_ERROR_FAILURE;
    }

    init_nvs = true;
    ESP_LOGI(tag, "NVS initialized");

#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_NVS_ENABLE */

    /* LittleFS */

#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_LITTLEFS_ENABLE

    esp_vfs_littlefs_conf_t littlefs_cfg = {
        .base_path              = cmp_main_config.driver.littlefs_base_path,
        .partition_label        = cmp_main_config.driver.littlefs_partition_label,
        .partition              = NULL,
        .format_if_mount_failed = cmp_main_config.driver.littlefs_format_if_failed,
        .read_only              = 0,
        .dont_mount             = 0,
        .grow_on_mount          = 0,
    };

    err = esp_vfs_littlefs_register(&littlefs_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(tag, "Failed to initialize LittleFS: %s", esp_err_to_name(err));
        cmp_main_driver_deinit(launcher);
        return DOMAIN_MODELS_ERROR_FAILURE;
    }

    init_littlefs = true;
    ESP_LOGI(tag, "LittleFS initialized");

#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_LITTLEFS_ENABLE */

    /* SD Card */

#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_SD_CARD_ENABLE
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_SD_CARD_USE_SDSPI

    sdmmc_host_t sd_card_sdspi_host_cfg = SDSPI_HOST_DEFAULT();
    sd_card_sdspi_host_cfg.slot         = cmp_main_config.driver.sd_card_sdspi_spi_host;
    sd_card_sdspi_host_cfg.max_freq_khz = cmp_main_config.driver.sd_card_sdspi_max_freq_khz;

    sdspi_device_config_t sd_card_sdspi_dev_cfg = SDSPI_DEVICE_CONFIG_DEFAULT();
    sd_card_sdspi_dev_cfg.host_id               = cmp_main_config.driver.sd_card_sdspi_spi_host;
    sd_card_sdspi_dev_cfg.gpio_cs               = cmp_main_config.driver.sd_card_sdspi_cs_pin;

    esp_vfs_fat_mount_config_t sd_card_mount_cfg = VFS_FAT_MOUNT_DEFAULT_CONFIG();
    sd_card_mount_cfg.format_if_mount_failed     = cmp_main_config.driver.sd_card_format_if_mount_failed;
    sd_card_mount_cfg.max_files                  = cmp_main_config.driver.sd_card_max_files;
    sd_card_mount_cfg.allocation_unit_size       = cmp_main_config.driver.sd_card_allocation_unit_size;

    err = esp_vfs_fat_sdspi_mount(
        cmp_main_config.driver.sd_card_base_path,
        &sd_card_sdspi_host_cfg,
        &sd_card_sdspi_dev_cfg,
        &sd_card_mount_cfg,
        &launcher->driver.sd_card_sdspi_card
    );
    if (err != ESP_OK) {
        ESP_LOGE(tag, "Failed to mount SDSPI card: %s", esp_err_to_name(err));
        cmp_main_driver_deinit(launcher);
        return DOMAIN_MODELS_ERROR_FAILURE;
    }

    init_sd_card_sdspi = true;
    ESP_LOGI(tag, "SDSPI card mounted at %s", cmp_main_config.driver.sd_card_base_path);

#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_SD_CARD_USE_SDSPI */
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_SD_CARD_ENABLE */

    /* Event Loop */

#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_EVENT_LOOP_ENABLE

    err = esp_event_loop_create_default();
    if (err != ESP_OK) {
        ESP_LOGE(tag, "Failed to create event loop: %s", esp_err_to_name(err));
        cmp_main_driver_deinit(launcher);
        return DOMAIN_MODELS_ERROR_FAILURE;
    }

    init_event_loop = true;
    ESP_LOGI(tag, "Event loop created");

#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_EVENT_LOOP_ENABLE */

    /* ESP Netif */

#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_NETIF_ENABLE

    err = esp_netif_init();
    if (err != ESP_OK) {
        ESP_LOGE(tag, "Failed to initialize ESP Netif: %s", esp_err_to_name(err));
        cmp_main_driver_deinit(launcher);
        return DOMAIN_MODELS_ERROR_FAILURE;
    }

    init_netif = true;
    ESP_LOGI(tag, "ESP Netif initialized");

#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_NETIF_ENABLE */

    /* WiFi */

#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_WIFI_ENABLE

    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();

    err = esp_wifi_init(&wifi_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(tag, "Failed to initialize WiFi: %s", esp_err_to_name(err));
        cmp_main_driver_deinit(launcher);
        return DOMAIN_MODELS_ERROR_FAILURE;
    }

    init_wifi = true;
    ESP_LOGI(tag, "WiFi initialized");

#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_WIFI_ENABLE */

    /* Ethernet */

#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_ETHERNET_ENABLE
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_ETHERNET_USE_W5500

    spi_device_interface_config_t ethernet_w5500_spi_dev_cfg;
    memset(&ethernet_w5500_spi_dev_cfg, 0, sizeof(spi_device_interface_config_t));
    ethernet_w5500_spi_dev_cfg.mode           = cmp_main_config.driver.ethernet_w5500_spi_mode;
    ethernet_w5500_spi_dev_cfg.clock_speed_hz = cmp_main_config.driver.ethernet_w5500_spi_clock_hz;
    ethernet_w5500_spi_dev_cfg.spics_io_num   = cmp_main_config.driver.ethernet_w5500_cs_pin;
    ethernet_w5500_spi_dev_cfg.queue_size     = cmp_main_config.driver.ethernet_w5500_spi_queue_size;

    eth_w5500_config_t ethernet_w5500_cfg = ETH_W5500_DEFAULT_CONFIG(
        cmp_main_config.driver.ethernet_w5500_spi_host,
        &ethernet_w5500_spi_dev_cfg
    );
    ethernet_w5500_cfg.base.int_gpio_num   = cmp_main_config.driver.ethernet_w5500_int_pin;
    ethernet_w5500_cfg.base.poll_period_ms = cmp_main_config.driver.ethernet_w5500_poll_period_ms;

    eth_mac_config_t ethernet_w5500_mac_config   = ETH_MAC_DEFAULT_CONFIG();
    ethernet_w5500_mac_config.rx_task_stack_size = cmp_main_config.driver.ethernet_w5500_rx_task_stack_size;

    eth_phy_config_t ethernet_w5500_phy_config = ETH_PHY_DEFAULT_CONFIG();
    ethernet_w5500_phy_config.reset_gpio_num   = cmp_main_config.driver.ethernet_w5500_rst_pin;

    launcher->driver.ethernet_w5500_mac = esp_eth_mac_new_w5500(&ethernet_w5500_cfg, &ethernet_w5500_mac_config);
    if (!launcher->driver.ethernet_w5500_mac) {
        ESP_LOGE(tag, "Failed to initialize W5500 MAC: %s", esp_err_to_name(err));
        cmp_main_driver_deinit(launcher);
        return DOMAIN_MODELS_ERROR_MALLOC_FAILED;
    }

    init_ethernet_w5500_mac = true;

    launcher->driver.ethernet_w5500_phy = esp_eth_phy_new_w5500(&ethernet_w5500_phy_config);
    if (!launcher->driver.ethernet_w5500_phy) {
        ESP_LOGE(tag, "Failed to initialize W5500 PHY: %s", esp_err_to_name(err));
        cmp_main_driver_deinit(launcher);
        return DOMAIN_MODELS_ERROR_MALLOC_FAILED;
    }

    init_ethernet_w5500_phy = true;

    esp_eth_config_t ethernet_cfg = ETH_DEFAULT_CONFIG(
        launcher->driver.ethernet_w5500_mac,
        launcher->driver.ethernet_w5500_phy
    );

    err = esp_eth_driver_install(&ethernet_cfg, &launcher->driver.ethernet_w5500_handle);
    if (err != ESP_OK) {
        ESP_LOGE(tag, "Failed to install W5500 ethernet driver: %s", esp_err_to_name(err));
        cmp_main_driver_deinit(launcher);
        return DOMAIN_MODELS_ERROR_FAILURE;
    }

    init_ethernet_w5500 = true;

#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_ETHERNET_USE_W5500 */
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_ETHERNET_ENABLE */

    /* BLE */

#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_BLE_ENABLE

    err = nimble_port_init();
    if (err != ESP_OK) {
        ESP_LOGE(tag, "Failed to initialize BLE port: %s", esp_err_to_name(err));
        cmp_main_driver_deinit(launcher);
        return DOMAIN_MODELS_ERROR_FAILURE;
    }

    ble_svc_gap_init();
    ble_svc_gatt_init();

    int rc = ble_svc_gap_device_name_set(cmp_main_config.driver.ble_device_name);
    if (rc != 0) {
        ESP_LOGE(tag, "Failed to set BLE device name: %d", rc);
        cmp_main_driver_deinit(launcher);
        return DOMAIN_MODELS_ERROR_FAILURE;
    }

    init_ble = true;
    ESP_LOGI(tag, "BLE initialized");

#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_BLE_ENABLE */

    return DOMAIN_MODELS_ERROR_OK;
}

void cmp_main_driver_deinit(cmp_main_launcher_t* launcher) {
    if (init_mqtt_client) {
        init_mqtt_client = false;
    }
    if (init_http_server) {
        init_http_server = false;
    }
    if (init_ble) {
        ble_svc_gatt_deinit();
        ble_svc_gap_deinit();
        nimble_port_deinit();
        init_ble = false;
    }
    if (init_ethernet_w5500) {
        esp_eth_driver_uninstall(launcher->driver.ethernet_w5500_handle);
        init_ethernet_w5500 = false;
    }
    if (init_ethernet_w5500_phy) {
        launcher->driver.ethernet_w5500_phy->del(launcher->driver.ethernet_w5500_phy);
        init_ethernet_w5500_phy = false;
    }
    if (init_ethernet_w5500_mac) {
        launcher->driver.ethernet_w5500_mac->del(launcher->driver.ethernet_w5500_mac);
        init_ethernet_w5500_mac = false;
    }
    if (init_wifi) {
        esp_wifi_deinit();
        init_wifi = false;
    }
    if (init_netif) {
        esp_netif_deinit();
        init_netif = false;
    }
    if (init_event_loop) {
        esp_event_loop_delete_default();
        init_event_loop = false;
    }
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_SD_CARD_ENABLE
#ifdef COMPOSITION_MAIN_CONFIG_DRIVER_SD_CARD_USE_SDSPI
    if (init_sd_card_sdspi) {
        esp_vfs_fat_sdcard_unmount(
            cmp_main_config.driver.sd_card_base_path,
            launcher->driver.sd_card_sdspi_card
        );
        init_sd_card_sdspi = false;
    }
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_SD_CARD_USE_SDSPI */
#endif /* COMPOSITION_MAIN_CONFIG_DRIVER_SD_CARD_ENABLE */
    if (init_littlefs) {
        esp_vfs_littlefs_unregister(cmp_main_config.driver.littlefs_partition_label);
        init_littlefs = false;
    }
    if (init_nvs) {
        nvs_flash_deinit();
        init_nvs = false;
    }
    if (init_spi_3) {
        spi_bus_free(SPI3_HOST);
        init_spi_3 = false;
    }
    if (init_spi_2) {
        spi_bus_free(SPI2_HOST);
        init_spi_2 = false;
    }
    if (init_i2c_1) {
        i2c_del_master_bus(launcher->driver.i2c_1_bus_handle);
        init_i2c_1 = false;
    }
    if (init_i2c_0) {
        i2c_del_master_bus(launcher->driver.i2c_0_bus_handle);
        init_i2c_0 = false;
    }
    if (init_isr) {
        gpio_uninstall_isr_service();
        init_isr = false;
    }
}
