#include "infrastructure/device/wifi/stub_impl_utils.h"

#include <string.h>

dom_models_error_t inf_device_wifi_stub_impl_bad_argument_error(void) {
    return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
}

bool inf_device_wifi_stub_impl_valid_mode(dom_models_wifi_mode_t mode) {
    switch (mode) {
        case DOM_MODELS_WIFI_MODE_NULL:
        case DOM_MODELS_WIFI_MODE_STA:
        case DOM_MODELS_WIFI_MODE_AP:
        case DOM_MODELS_WIFI_MODE_APSTA:
        case DOM_MODELS_WIFI_MODE_NAN:
            return true;
        case DOM_MODELS_WIFI_MODE_OTHER:
            return false;
    }

    return false;
}

size_t inf_device_wifi_stub_impl_bounded_strlen(const char* value, size_t max_len) {
    size_t len = 0;

    if (!value) {
        return 0;
    }

    while (len < max_len && value[len] != '\0') {
        len++;
    }

    return len;
}

void inf_device_wifi_stub_impl_copy_cstr(char* dst, size_t dst_size, const char* src) {
    if (!dst || dst_size == 0) {
        return;
    }

    const char* value = src ? src : "";
    size_t      len   = inf_device_wifi_stub_impl_bounded_strlen(value, dst_size - 1);
    dst[len]          = '\0';

    if (len > 0) {
        memcpy(dst, value, len);
    }
}

void inf_device_wifi_stub_impl_copy_if_key(bool* available, char* out, size_t out_size, const char* if_key) {
    if (!available || !out || out_size == 0) {
        return;
    }

    if (!if_key || if_key[0] == '\0') {
        *available = false;
        out[0]     = '\0';
        return;
    }

    *available = true;
    inf_device_wifi_stub_impl_copy_cstr(out, out_size, if_key);
}

void inf_device_wifi_stub_impl_copy_mac(uint8_t dst[DOM_MODELS_WIFI_MAC_LEN], const uint8_t src[DOM_MODELS_WIFI_MAC_LEN]) {
    if (!dst || !src) {
        return;
    }

    memcpy(dst, src, DOM_MODELS_WIFI_MAC_LEN);
}

const char* inf_device_wifi_stub_impl_default_ssid(inf_device_wifi_stub_impl_ctx_t* ctx) {
    if (!ctx || !ctx->cfg.default_ssid || ctx->cfg.default_ssid[0] == '\0') {
        return "haya-stub";
    }

    return ctx->cfg.default_ssid;
}

void inf_device_wifi_stub_impl_fill_ap_record(dom_models_wifi_ap_record_t* out, const char* ssid, const uint8_t bssid[DOM_MODELS_WIFI_MAC_LEN], uint8_t channel, int8_t rssi, dom_models_wifi_auth_mode_t auth_mode) {
    if (!out) {
        return;
    }

    memset(out, 0, sizeof(dom_models_wifi_ap_record_t));

    out->bssid_available = true;
    inf_device_wifi_stub_impl_copy_mac(out->bssid, bssid);
    inf_device_wifi_stub_impl_copy_cstr(out->ssid, sizeof(out->ssid), ssid);
    out->primary_channel = channel;
    out->second_channel  = DOM_MODELS_WIFI_SECOND_CHANNEL_NONE;
    out->rssi            = rssi;
    out->auth_mode       = auth_mode;
    out->pairwise_cipher = DOM_MODELS_WIFI_CIPHER_CCMP;
    out->group_cipher    = DOM_MODELS_WIFI_CIPHER_CCMP;
    out->bandwidth       = DOM_MODELS_WIFI_BANDWIDTH_20MHZ;
    out->phy_flags       = DOM_MODELS_WIFI_PHY_FLAG_11B | DOM_MODELS_WIFI_PHY_FLAG_11G | DOM_MODELS_WIFI_PHY_FLAG_11N;
}

void inf_device_wifi_stub_impl_fill_default_connected_ap(inf_device_wifi_stub_impl_ctx_t* ctx, const char* ssid, uint8_t channel, dom_models_wifi_auth_mode_t auth_mode) {
    static const uint8_t bssid[DOM_MODELS_WIFI_MAC_LEN] = {0x02, 0x00, 0x00, 0x00, 0x10, 0x01};

    if (!ctx) {
        return;
    }

    inf_device_wifi_stub_impl_fill_ap_record(&ctx->connected_ap, ssid, bssid, channel, -42, auth_mode);
}

void inf_device_wifi_stub_impl_fill_default_scan(inf_device_wifi_stub_impl_ctx_t* ctx, const char* ssid, uint8_t channel) {
    static const uint8_t bssid[DOM_MODELS_WIFI_MAC_LEN] = {0x02, 0x00, 0x00, 0x00, 0x10, 0x01};
    uint32_t             scan_id = 0;

    if (!ctx) {
        return;
    }

    scan_id = ctx->scanned.scan_id;

    memset(&ctx->scanned, 0, sizeof(dom_models_wifi_scan_result_t));

    ctx->scanned.status        = DOM_MODELS_WIFI_SCAN_STATUS_DONE;
    ctx->scanned.scan_id       = scan_id;
    ctx->scanned.driver_status = 0;
    ctx->scanned.total_count   = 1;
    ctx->scanned.count         = 1;
    ctx->scanned.truncated     = false;

    inf_device_wifi_stub_impl_fill_ap_record(&ctx->scanned.records[0], ssid, bssid, channel, -42, DOM_MODELS_WIFI_AUTH_WPA2_PSK);
}

void inf_device_wifi_stub_impl_reset_runtime(inf_device_wifi_stub_impl_ctx_t* ctx) {
    if (!ctx) {
        return;
    }

    ctx->started    = false;
    ctx->connected  = false;
    ctx->ap_started = false;
    ctx->mode       = DOM_MODELS_WIFI_MODE_NULL;

    inf_device_wifi_stub_impl_fill_default_connected_ap(ctx, inf_device_wifi_stub_impl_default_ssid(ctx), 1, DOM_MODELS_WIFI_AUTH_WPA2_PSK);
    inf_device_wifi_stub_impl_fill_default_scan(ctx, inf_device_wifi_stub_impl_default_ssid(ctx), 1);
}
