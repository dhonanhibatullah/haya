#include "infrastructure/device/ethernet/stub_impl_utils.h"

#include <string.h>

size_t inf_device_ethernet_stub_impl_bounded_strlen(const char* value, size_t max_len) {
    size_t len = 0;

    if (!value) {
        return 0;
    }

    while (len < max_len && value[len] != '\0') {
        len++;
    }

    return len;
}

void inf_device_ethernet_stub_impl_copy_cstr(char* dst, size_t dst_size, const char* src) {
    if (!dst || dst_size == 0) {
        return;
    }

    const char* value = src ? src : "";
    size_t      len   = inf_device_ethernet_stub_impl_bounded_strlen(value, dst_size - 1);
    dst[len]          = '\0';

    if (len > 0) {
        memcpy(dst, value, len);
    }
}

void inf_device_ethernet_stub_impl_copy_if_key(bool* available, char* out, size_t out_size, const char* if_key) {
    if (!available || !out || out_size == 0) {
        return;
    }

    if (!if_key || if_key[0] == '\0') {
        *available = false;
        out[0]     = '\0';
        return;
    }

    *available = true;
    inf_device_ethernet_stub_impl_copy_cstr(out, out_size, if_key);
}

void inf_device_ethernet_stub_impl_copy_mac(uint8_t dst[DOM_MODELS_ETHERNET_MAC_LEN], const uint8_t src[DOM_MODELS_ETHERNET_MAC_LEN]) {
    if (!dst || !src) {
        return;
    }

    memcpy(dst, src, DOM_MODELS_ETHERNET_MAC_LEN);
}

bool inf_device_ethernet_stub_impl_valid_fixed_link_config(const dom_models_ethernet_link_config_t* config) {
    if (!config) {
        return false;
    }

    if (config->autoneg) {
        return true;
    }

    if (config->speed != DOM_MODELS_ETHERNET_SPEED_10M &&
        config->speed != DOM_MODELS_ETHERNET_SPEED_100M &&
        config->speed != DOM_MODELS_ETHERNET_SPEED_1000M) {
        return false;
    }

    return config->duplex == DOM_MODELS_ETHERNET_DUPLEX_HALF ||
           config->duplex == DOM_MODELS_ETHERNET_DUPLEX_FULL;
}
