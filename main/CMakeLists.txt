file(GLOB_RECURSE srcsc "src/*.c")
idf_component_register(
    SRCS 
        "src/main.c"
        ${srcsc}
    INCLUDE_DIRS 
        "include"
    PRIV_REQUIRES
        freertos
        esp_driver_i2c
        esp_driver_spi
        esp_driver_uart
        esp_driver_twai
        esp_driver_gpio
        fatfs
        littlefs
        nvs_flash
        esp_netif
        esp_wifi
        esp_eth
        w5500
        esp_http_server
        esp_http_client
        mqtt
        app_update
)

target_add_binary_data(${COMPONENT_TARGET} "include/haya/service/http_server/html/404_min.html" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "include/haya/service/wifiman/html/wifiman_min.html" TEXT)