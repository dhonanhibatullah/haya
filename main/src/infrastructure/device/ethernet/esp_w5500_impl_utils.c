#include "infrastructure/device/ethernet/esp_w5500_impl_utils.h"

#include <string.h>

dom_models_error_t inf_device_ethernet_esp_w5500_impl_error_from_esp(esp_err_t err) {
    switch (err) {
        case ESP_OK:
            return DOMAIN_MODELS_ERROR_OK;
        case ESP_ERR_NO_MEM:
            return DOMAIN_MODELS_ERROR_MALLOC_FAILED;
        case ESP_ERR_INVALID_ARG:
            return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
        case ESP_ERR_INVALID_STATE:
        case ESP_ERR_INVALID_SIZE:
            return DOMAIN_MODELS_ERROR_BAD_STATE;
        case ESP_ERR_NOT_SUPPORTED:
            return DOMAIN_MODELS_ERROR_NOT_SUPPORTED;
        case ESP_ERR_TIMEOUT:
            return DOMAIN_MODELS_ERROR_TIMEOUT;
        default:
            return DOMAIN_MODELS_ERROR_FAILURE;
    }
}

size_t inf_device_ethernet_esp_w5500_impl_bounded_strlen(const char* value, size_t max_len) {
    size_t len = 0;

    if (!value) {
        return 0;
    }

    while (len < max_len && value[len] != '\0') {
        len++;
    }

    return len;
}

void inf_device_ethernet_esp_w5500_impl_copy_cstr(char* dst, size_t dst_size, const char* src) {
    if (!dst || dst_size == 0) {
        return;
    }

    const char* value = src ? src : "";
    size_t      len   = inf_device_ethernet_esp_w5500_impl_bounded_strlen(value, dst_size - 1);
    dst[len]          = '\0';

    if (len > 0) {
        memcpy(dst, value, len);
    }
}

void inf_device_ethernet_esp_w5500_impl_copy_if_key(bool* available, char* out, size_t out_size, const char* if_key) {
    if (!available || !out || out_size == 0) {
        return;
    }

    if (!if_key || if_key[0] == '\0') {
        *available = false;
        out[0]     = '\0';
        return;
    }

    *available = true;
    inf_device_ethernet_esp_w5500_impl_copy_cstr(out, out_size, if_key);
}

bool inf_device_ethernet_esp_w5500_impl_speed_from_domain(dom_models_ethernet_speed_t in, eth_speed_t* out) {
    if (!out) {
        return false;
    }

    switch (in) {
        case DOM_MODELS_ETHERNET_SPEED_10M:
            *out = ETH_SPEED_10M;
            return true;
        case DOM_MODELS_ETHERNET_SPEED_100M:
            *out = ETH_SPEED_100M;
            return true;
        default:
            return false;
    }
}

dom_models_ethernet_speed_t inf_device_ethernet_esp_w5500_impl_speed_to_domain(eth_speed_t in) {
    switch (in) {
        case ETH_SPEED_10M:
            return DOM_MODELS_ETHERNET_SPEED_10M;
        case ETH_SPEED_100M:
            return DOM_MODELS_ETHERNET_SPEED_100M;
        default:
            return DOM_MODELS_ETHERNET_SPEED_UNKNOWN;
    }
}

bool inf_device_ethernet_esp_w5500_impl_duplex_from_domain(dom_models_ethernet_duplex_t in, eth_duplex_t* out) {
    if (!out) {
        return false;
    }

    switch (in) {
        case DOM_MODELS_ETHERNET_DUPLEX_HALF:
            *out = ETH_DUPLEX_HALF;
            return true;
        case DOM_MODELS_ETHERNET_DUPLEX_FULL:
            *out = ETH_DUPLEX_FULL;
            return true;
        default:
            return false;
    }
}

dom_models_ethernet_duplex_t inf_device_ethernet_esp_w5500_impl_duplex_to_domain(eth_duplex_t in) {
    switch (in) {
        case ETH_DUPLEX_HALF:
            return DOM_MODELS_ETHERNET_DUPLEX_HALF;
        case ETH_DUPLEX_FULL:
            return DOM_MODELS_ETHERNET_DUPLEX_FULL;
        default:
            return DOM_MODELS_ETHERNET_DUPLEX_UNKNOWN;
    }
}
