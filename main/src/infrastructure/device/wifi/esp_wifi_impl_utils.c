#include "infrastructure/device/wifi/esp_wifi_impl_utils.h"

#include <string.h>

#include "esp_wifi.h"

dom_models_error_t inf_device_wifi_esp_wifi_impl_error_from_esp(esp_err_t err) {
    switch (err) {
        case ESP_OK:
            return DOMAIN_MODELS_ERROR_OK;
        case ESP_ERR_NO_MEM:
            return DOMAIN_MODELS_ERROR_MALLOC_FAILED;
        case ESP_ERR_INVALID_ARG:
            return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
        case ESP_ERR_INVALID_STATE:
        case ESP_ERR_WIFI_NOT_INIT:
        case ESP_ERR_WIFI_NOT_STARTED:
        case ESP_ERR_WIFI_IF:
            return DOMAIN_MODELS_ERROR_BAD_STATE;
        case ESP_ERR_NOT_SUPPORTED:
            return DOMAIN_MODELS_ERROR_NOT_SUPPORTED;
        case ESP_ERR_TIMEOUT:
            return DOMAIN_MODELS_ERROR_TIMEOUT;
        default:
            return DOMAIN_MODELS_ERROR_FAILURE;
    }
}

size_t inf_device_wifi_esp_wifi_impl_bounded_strlen(const char* value, size_t max_len) {
    size_t len = 0;

    if (!value) {
        return 0;
    }

    while (len < max_len && value[len] != '\0') {
        len++;
    }

    return len;
}

size_t inf_device_wifi_esp_wifi_impl_bounded_byte_strlen(const uint8_t* value, size_t max_len) {
    size_t len = 0;

    if (!value) {
        return 0;
    }

    while (len < max_len && value[len] != '\0') {
        len++;
    }

    return len;
}

void inf_device_wifi_esp_wifi_impl_copy_cstr(char* dst, size_t dst_size, const char* src) {
    if (!dst || dst_size == 0) {
        return;
    }

    const char* value = src ? src : "";
    size_t      len   = inf_device_wifi_esp_wifi_impl_bounded_strlen(value, dst_size - 1);
    dst[len]          = '\0';

    if (len > 0) {
        memcpy(dst, value, len);
    }
}

void inf_device_wifi_esp_wifi_impl_copy_cstr_to_bytes(uint8_t* dst, size_t dst_size, const char* src) {
    if (!dst || dst_size == 0 || !src) {
        return;
    }

    size_t len = inf_device_wifi_esp_wifi_impl_bounded_strlen(src, dst_size);
    if (len > 0) {
        memcpy(dst, src, len);
    }
}

void inf_device_wifi_esp_wifi_impl_copy_bytes_to_cstr(char* dst, size_t dst_size, const uint8_t* src, size_t src_size) {
    if (!dst || dst_size == 0) {
        return;
    }

    size_t len      = inf_device_wifi_esp_wifi_impl_bounded_byte_strlen(src, src_size);
    size_t copy_len = len < dst_size - 1 ? len : dst_size - 1;
    dst[copy_len]   = '\0';

    if (copy_len > 0) {
        memcpy(dst, src, copy_len);
    }
}

bool inf_device_wifi_esp_wifi_impl_wifi_mode_from_domain(dom_models_wifi_mode_t in, wifi_mode_t* out) {
    if (!out) {
        return false;
    }

    switch (in) {
        case DOM_MODELS_WIFI_MODE_NULL:
            *out = WIFI_MODE_NULL;
            return true;
        case DOM_MODELS_WIFI_MODE_STA:
            *out = WIFI_MODE_STA;
            return true;
        case DOM_MODELS_WIFI_MODE_AP:
            *out = WIFI_MODE_AP;
            return true;
        case DOM_MODELS_WIFI_MODE_APSTA:
            *out = WIFI_MODE_APSTA;
            return true;
        case DOM_MODELS_WIFI_MODE_NAN:
            *out = WIFI_MODE_NAN;
            return true;
        case DOM_MODELS_WIFI_MODE_OTHER:
            return false;
    }

    return false;
}

dom_models_wifi_mode_t inf_device_wifi_esp_wifi_impl_wifi_mode_to_domain(wifi_mode_t in) {
    switch (in) {
        case WIFI_MODE_NULL:
            return DOM_MODELS_WIFI_MODE_NULL;
        case WIFI_MODE_STA:
            return DOM_MODELS_WIFI_MODE_STA;
        case WIFI_MODE_AP:
            return DOM_MODELS_WIFI_MODE_AP;
        case WIFI_MODE_APSTA:
            return DOM_MODELS_WIFI_MODE_APSTA;
        case WIFI_MODE_NAN:
            return DOM_MODELS_WIFI_MODE_NAN;
        default:
            return DOM_MODELS_WIFI_MODE_OTHER;
    }
}

bool inf_device_wifi_esp_wifi_impl_wifi_auth_from_domain(dom_models_wifi_auth_mode_t in, wifi_auth_mode_t* out) {
    if (!out) {
        return false;
    }

    switch (in) {
        case DOM_MODELS_WIFI_AUTH_OPEN:
            *out = WIFI_AUTH_OPEN;
            return true;
        case DOM_MODELS_WIFI_AUTH_WEP:
            *out = WIFI_AUTH_WEP;
            return true;
        case DOM_MODELS_WIFI_AUTH_WPA_PSK:
            *out = WIFI_AUTH_WPA_PSK;
            return true;
        case DOM_MODELS_WIFI_AUTH_WPA2_PSK:
            *out = WIFI_AUTH_WPA2_PSK;
            return true;
        case DOM_MODELS_WIFI_AUTH_WPA_WPA2_PSK:
            *out = WIFI_AUTH_WPA_WPA2_PSK;
            return true;
        case DOM_MODELS_WIFI_AUTH_WPA_ENTERPRISE:
            *out = WIFI_AUTH_WPA_ENTERPRISE;
            return true;
        case DOM_MODELS_WIFI_AUTH_WPA2_ENTERPRISE:
            *out = WIFI_AUTH_WPA2_ENTERPRISE;
            return true;
        case DOM_MODELS_WIFI_AUTH_WPA3_PSK:
            *out = WIFI_AUTH_WPA3_PSK;
            return true;
        case DOM_MODELS_WIFI_AUTH_WPA2_WPA3_PSK:
            *out = WIFI_AUTH_WPA2_WPA3_PSK;
            return true;
        case DOM_MODELS_WIFI_AUTH_WAPI_PSK:
            *out = WIFI_AUTH_WAPI_PSK;
            return true;
        case DOM_MODELS_WIFI_AUTH_OWE:
            *out = WIFI_AUTH_OWE;
            return true;
        case DOM_MODELS_WIFI_AUTH_WPA3_ENT_192:
            *out = WIFI_AUTH_WPA3_ENT_192;
            return true;
        case DOM_MODELS_WIFI_AUTH_DPP:
            *out = WIFI_AUTH_DPP;
            return true;
        case DOM_MODELS_WIFI_AUTH_WPA3_ENTERPRISE:
            *out = WIFI_AUTH_WPA3_ENTERPRISE;
            return true;
        case DOM_MODELS_WIFI_AUTH_WPA2_WPA3_ENTERPRISE:
            *out = WIFI_AUTH_WPA2_WPA3_ENTERPRISE;
            return true;
        case DOM_MODELS_WIFI_AUTH_OTHER:
        case DOM_MODELS_WIFI_AUTH_UNKNOWN:
            return false;
    }

    return false;
}

dom_models_wifi_auth_mode_t inf_device_wifi_esp_wifi_impl_wifi_auth_to_domain(wifi_auth_mode_t in) {
    switch (in) {
        case WIFI_AUTH_OPEN:
            return DOM_MODELS_WIFI_AUTH_OPEN;
        case WIFI_AUTH_WEP:
            return DOM_MODELS_WIFI_AUTH_WEP;
        case WIFI_AUTH_WPA_PSK:
            return DOM_MODELS_WIFI_AUTH_WPA_PSK;
        case WIFI_AUTH_WPA2_PSK:
            return DOM_MODELS_WIFI_AUTH_WPA2_PSK;
        case WIFI_AUTH_WPA_WPA2_PSK:
            return DOM_MODELS_WIFI_AUTH_WPA_WPA2_PSK;
        case WIFI_AUTH_WPA_ENTERPRISE:
            return DOM_MODELS_WIFI_AUTH_WPA_ENTERPRISE;
        case WIFI_AUTH_WPA2_ENTERPRISE:
            return DOM_MODELS_WIFI_AUTH_WPA2_ENTERPRISE;
        case WIFI_AUTH_WPA3_PSK:
            return DOM_MODELS_WIFI_AUTH_WPA3_PSK;
        case WIFI_AUTH_WPA2_WPA3_PSK:
            return DOM_MODELS_WIFI_AUTH_WPA2_WPA3_PSK;
        case WIFI_AUTH_WAPI_PSK:
            return DOM_MODELS_WIFI_AUTH_WAPI_PSK;
        case WIFI_AUTH_OWE:
            return DOM_MODELS_WIFI_AUTH_OWE;
        case WIFI_AUTH_WPA3_ENT_192:
            return DOM_MODELS_WIFI_AUTH_WPA3_ENT_192;
        case WIFI_AUTH_DPP:
            return DOM_MODELS_WIFI_AUTH_DPP;
        case WIFI_AUTH_WPA3_ENTERPRISE:
            return DOM_MODELS_WIFI_AUTH_WPA3_ENTERPRISE;
        case WIFI_AUTH_WPA2_WPA3_ENTERPRISE:
            return DOM_MODELS_WIFI_AUTH_WPA2_WPA3_ENTERPRISE;
        default:
            return DOM_MODELS_WIFI_AUTH_OTHER;
    }
}

dom_models_wifi_cipher_t inf_device_wifi_esp_wifi_impl_wifi_cipher_to_domain(wifi_cipher_type_t in) {
    switch (in) {
        case WIFI_CIPHER_TYPE_NONE:
            return DOM_MODELS_WIFI_CIPHER_NONE;
        case WIFI_CIPHER_TYPE_WEP40:
            return DOM_MODELS_WIFI_CIPHER_WEP40;
        case WIFI_CIPHER_TYPE_WEP104:
            return DOM_MODELS_WIFI_CIPHER_WEP104;
        case WIFI_CIPHER_TYPE_TKIP:
            return DOM_MODELS_WIFI_CIPHER_TKIP;
        case WIFI_CIPHER_TYPE_CCMP:
            return DOM_MODELS_WIFI_CIPHER_CCMP;
        case WIFI_CIPHER_TYPE_TKIP_CCMP:
            return DOM_MODELS_WIFI_CIPHER_TKIP_CCMP;
        case WIFI_CIPHER_TYPE_AES_CMAC128:
            return DOM_MODELS_WIFI_CIPHER_AES_CMAC128;
        case WIFI_CIPHER_TYPE_SMS4:
            return DOM_MODELS_WIFI_CIPHER_SMS4;
        case WIFI_CIPHER_TYPE_GCMP:
            return DOM_MODELS_WIFI_CIPHER_GCMP;
        case WIFI_CIPHER_TYPE_GCMP256:
            return DOM_MODELS_WIFI_CIPHER_GCMP256;
        case WIFI_CIPHER_TYPE_AES_GMAC128:
            return DOM_MODELS_WIFI_CIPHER_AES_GMAC128;
        case WIFI_CIPHER_TYPE_AES_GMAC256:
            return DOM_MODELS_WIFI_CIPHER_AES_GMAC256;
        default:
            return DOM_MODELS_WIFI_CIPHER_OTHER;
    }
}

dom_models_wifi_bandwidth_t inf_device_wifi_esp_wifi_impl_wifi_bandwidth_to_domain(wifi_bandwidth_t in) {
    switch (in) {
        case WIFI_BW20:
            return DOM_MODELS_WIFI_BANDWIDTH_20MHZ;
        case WIFI_BW40:
            return DOM_MODELS_WIFI_BANDWIDTH_40MHZ;
        case WIFI_BW80:
            return DOM_MODELS_WIFI_BANDWIDTH_80MHZ;
        case WIFI_BW160:
            return DOM_MODELS_WIFI_BANDWIDTH_160MHZ;
        case WIFI_BW80_BW80:
            return DOM_MODELS_WIFI_BANDWIDTH_80_80MHZ;
        default:
            return DOM_MODELS_WIFI_BANDWIDTH_OTHER;
    }
}

dom_models_wifi_second_channel_t inf_device_wifi_esp_wifi_impl_wifi_second_channel_to_domain(wifi_second_chan_t in) {
    switch (in) {
        case WIFI_SECOND_CHAN_ABOVE:
            return DOM_MODELS_WIFI_SECOND_CHANNEL_ABOVE;
        case WIFI_SECOND_CHAN_BELOW:
            return DOM_MODELS_WIFI_SECOND_CHANNEL_BELOW;
        case WIFI_SECOND_CHAN_NONE:
            return DOM_MODELS_WIFI_SECOND_CHANNEL_NONE;
        default:
            return DOM_MODELS_WIFI_SECOND_CHANNEL_OTHER;
    }
}

uint32_t inf_device_wifi_esp_wifi_impl_phy_flags_from_ap_record(const wifi_ap_record_t* record) {
    uint32_t flags = 0;

    if (!record) {
        return flags;
    }

    if (record->phy_11b) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_11B;
    }
    if (record->phy_11g) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_11G;
    }
    if (record->phy_11n) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_11N;
    }
    if (record->phy_lr) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_LR;
    }
    if (record->phy_11a) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_11A;
    }
    if (record->phy_11ac) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_11AC;
    }
    if (record->phy_11ax) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_11AX;
    }
    if (record->wps) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_WPS;
    }
    if (record->ftm_responder) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_FTM_RESPONDER;
    }
    if (record->ftm_initiator) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_FTM_INITIATOR;
    }

    return flags;
}

uint32_t inf_device_wifi_esp_wifi_impl_phy_flags_from_sta_info(const wifi_sta_info_t* sta) {
    uint32_t flags = 0;

    if (!sta) {
        return flags;
    }

    if (sta->phy_11b) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_11B;
    }
    if (sta->phy_11g) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_11G;
    }
    if (sta->phy_11n) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_11N;
    }
    if (sta->phy_lr) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_LR;
    }
    if (sta->phy_11a) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_11A;
    }
    if (sta->phy_11ac) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_11AC;
    }
    if (sta->phy_11ax) {
        flags |= DOM_MODELS_WIFI_PHY_FLAG_11AX;
    }

    return flags;
}

void inf_device_wifi_esp_wifi_impl_copy_if_key(bool* available, char* out, size_t out_size, const char* if_key) {
    if (!available || !out || out_size == 0) {
        return;
    }

    if (!if_key || if_key[0] == '\0') {
        *available = false;
        out[0]     = '\0';
        return;
    }

    *available = true;
    inf_device_wifi_esp_wifi_impl_copy_cstr(out, out_size, if_key);
}

void inf_device_wifi_esp_wifi_impl_copy_ap_record(dom_models_wifi_ap_record_t* out, const wifi_ap_record_t* in) {
    if (!out || !in) {
        return;
    }

    memset(out, 0, sizeof(dom_models_wifi_ap_record_t));

    out->bssid_available = true;
    memcpy(out->bssid, in->bssid, sizeof(out->bssid));
    inf_device_wifi_esp_wifi_impl_copy_bytes_to_cstr(out->ssid, sizeof(out->ssid), in->ssid, sizeof(in->ssid));
    out->primary_channel = in->primary;
    out->second_channel  = inf_device_wifi_esp_wifi_impl_wifi_second_channel_to_domain(in->second);
    out->rssi            = in->rssi;
    out->auth_mode       = inf_device_wifi_esp_wifi_impl_wifi_auth_to_domain(in->authmode);
    out->pairwise_cipher = inf_device_wifi_esp_wifi_impl_wifi_cipher_to_domain(in->pairwise_cipher);
    out->group_cipher    = inf_device_wifi_esp_wifi_impl_wifi_cipher_to_domain(in->group_cipher);
    out->bandwidth       = inf_device_wifi_esp_wifi_impl_wifi_bandwidth_to_domain(in->bandwidth);
    out->phy_flags       = inf_device_wifi_esp_wifi_impl_phy_flags_from_ap_record(in);
}

void inf_device_wifi_esp_wifi_impl_copy_ap_client(dom_models_wifi_ap_client_t* out, const wifi_sta_info_t* in) {
    if (!out || !in) {
        return;
    }

    memset(out, 0, sizeof(dom_models_wifi_ap_client_t));

    memcpy(out->mac, in->mac, sizeof(out->mac));
    out->rssi      = in->rssi;
    out->phy_flags = inf_device_wifi_esp_wifi_impl_phy_flags_from_sta_info(in);
}

esp_err_t inf_device_wifi_esp_wifi_impl_ensure_sta_mode(void) {
    wifi_mode_t mode;
    esp_err_t   err = esp_wifi_get_mode(&mode);
    if (err != ESP_OK) {
        return err;
    }

    if (mode == WIFI_MODE_STA || mode == WIFI_MODE_APSTA) {
        return ESP_OK;
    }

    if (mode == WIFI_MODE_AP) {
        return esp_wifi_set_mode(WIFI_MODE_APSTA);
    }

    return esp_wifi_set_mode(WIFI_MODE_STA);
}

esp_err_t inf_device_wifi_esp_wifi_impl_ensure_ap_mode(void) {
    wifi_mode_t mode;
    esp_err_t   err = esp_wifi_get_mode(&mode);
    if (err != ESP_OK) {
        return err;
    }

    if (mode == WIFI_MODE_AP || mode == WIFI_MODE_APSTA) {
        return ESP_OK;
    }

    if (mode == WIFI_MODE_STA) {
        return esp_wifi_set_mode(WIFI_MODE_APSTA);
    }

    return esp_wifi_set_mode(WIFI_MODE_AP);
}

void inf_device_wifi_esp_wifi_impl_clear_scan_records(dom_models_wifi_scan_result_t* out) {
    if (!out) {
        return;
    }

    out->total_count = 0;
    out->count       = 0;
    out->truncated   = false;
    memset(out->records, 0, sizeof(out->records));
}

void inf_device_wifi_esp_wifi_impl_load_scan_records(inf_device_wifi_esp_wifi_impl_ctx_t* ctx) {
    if (!ctx) {
        return;
    }

    uint16_t  total = 0;
    esp_err_t err   = esp_wifi_scan_get_ap_num(&total);
    if (err != ESP_OK) {
        ctx->scanned.status        = DOM_MODELS_WIFI_SCAN_STATUS_FAILED;
        ctx->scanned.driver_status = (uint32_t)err;
        inf_device_wifi_esp_wifi_impl_clear_scan_records(&ctx->scanned);
        return;
    }

    ctx->scanned.total_count = total;
    ctx->scanned.count       = total;
    if (ctx->scanned.count > DOM_MODELS_WIFI_SCAN_RESULT_MAX) {
        ctx->scanned.count     = DOM_MODELS_WIFI_SCAN_RESULT_MAX;
        ctx->scanned.truncated = true;
    }

    if (ctx->scanned.count == 0) {
        return;
    }

    wifi_ap_record_t records[DOM_MODELS_WIFI_SCAN_RESULT_MAX];
    memset(records, 0, sizeof(records));

    uint16_t count = (uint16_t)ctx->scanned.count;
    err            = esp_wifi_scan_get_ap_records(&count, records);
    if (err != ESP_OK) {
        ctx->scanned.status        = DOM_MODELS_WIFI_SCAN_STATUS_FAILED;
        ctx->scanned.driver_status = (uint32_t)err;
        inf_device_wifi_esp_wifi_impl_clear_scan_records(&ctx->scanned);
        return;
    }

    ctx->scanned.count = count;
    for (size_t i = 0; i < ctx->scanned.count; i++) {
        inf_device_wifi_esp_wifi_impl_copy_ap_record(&ctx->scanned.records[i], &records[i]);
    }
}
