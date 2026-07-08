file(GLOB_RECURSE srcs "src/*.c" "src/*.cpp")

idf_component_register(
    SRCS 
        "main.c"
        ${srcs}
    INCLUDE_DIRS
        "include"
    PRIV_REQUIRES
        # built-in
        freertos
        spi_flash
        nvs_flash
        driver
        bt
        esp_timer
        esp_wifi
        esp_event
        esp_netif
        esp_http_server
        esp_https_ota
        app_update
        mbedtls
        mqtt
        # components
        # idf components
        cjson
        w5500
        littlefs
        esp_websocket_client
)

target_compile_definitions(${COMPONENT_LIB} PRIVATE PROJECT_NAME="${APP_NAME}")
