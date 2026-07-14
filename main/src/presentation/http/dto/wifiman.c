#include "presentation/http/dto/wifiman.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "cJSON.h"
#include "domain/models/error.h"
#include "domain/models/network.h"
#include "domain/models/wifi.h"
#include "domain/usecases/wifiman.h"

/* Helper Function Prototypes */

static size_t bounded_strlen(const char* value, size_t max_len);
static dom_models_error_t copy_json_string(cJSON* json, const char* key, char* out, size_t out_size, bool required);
static dom_models_error_t copy_optional_bool(cJSON* json, const char* key, bool* out);
static dom_models_error_t copy_optional_u8(cJSON* json, const char* key, uint8_t* out, bool* set, uint8_t min, uint8_t max);
static dom_models_error_t copy_optional_u32(cJSON* json, const char* key, uint32_t* out);
static bool parse_mac(const char* value, uint8_t out[DOM_MODELS_WIFI_MAC_LEN]);
static void mac_to_string(const uint8_t mac[DOM_MODELS_WIFI_MAC_LEN], char out[18]);
static void ipv4_to_string(const uint8_t value[DOM_MODELS_NETWORK_IPV4_LEN], char out[16]);
static void ipv6_to_string(const uint8_t value[DOM_MODELS_NETWORK_IPV6_LEN], char out[40]);
static const char* wifi_mode_to_string(dom_models_wifi_mode_t mode);
static const char* wifi_auth_to_string(dom_models_wifi_auth_mode_t auth_mode);
static const char* wifi_cipher_to_string(dom_models_wifi_cipher_t cipher);
static const char* wifi_bandwidth_to_string(dom_models_wifi_bandwidth_t bandwidth);
static const char* wifi_second_channel_to_string(dom_models_wifi_second_channel_t second_channel);
static const char* wifi_scan_status_to_string(dom_models_wifi_scan_status_t status);
static const char* network_interface_type_to_string(dom_models_network_interface_type_t type);
static const char* network_dhcp_status_to_string(dom_models_network_dhcp_status_t status);
static cJSON* ap_record_to_json(const dom_models_wifi_ap_record_t* record);
static cJSON* ap_client_to_json(const dom_models_wifi_ap_client_t* client);
static cJSON* wifi_status_to_json(const dom_models_wifi_status_t* status);
static cJSON* ipv4_info_to_json(const dom_models_network_ipv4_info_t* info);
static cJSON* ipv6_addr_to_json(const dom_models_network_ipv6_addr_t* info);
static cJSON* dns_info_to_json(const dom_models_network_dns_info_t* info);
static cJSON* network_interface_to_json(const dom_models_network_interface_t* interface);

dom_models_error_t pres_http_dto_wifiman_parse_sta_credential(
    cJSON*                            json,
    dom_models_wifi_sta_credential_t* out
) {
    if (!json || !out || !cJSON_IsObject(json)) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    memset(out, 0, sizeof(dom_models_wifi_sta_credential_t));

    dom_models_error_t err = copy_json_string(json, "ssid", out->ssid, sizeof(out->ssid), true);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    return copy_json_string(json, "password", out->password, sizeof(out->password), false);
}

dom_models_error_t pres_http_dto_wifiman_parse_scan_config(
    cJSON*                         json,
    dom_models_wifi_scan_config_t* out
) {
    if (!out) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    memset(out, 0, sizeof(dom_models_wifi_scan_config_t));

    if (!json) {
        return DOMAIN_MODELS_ERROR_OK;
    }
    if (!cJSON_IsObject(json)) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    dom_models_error_t err = copy_json_string(json, "ssid", out->ssid, sizeof(out->ssid), false);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }
    out->ssid_set = out->ssid[0] != '\0';

    cJSON* bssid = cJSON_GetObjectItemCaseSensitive(json, "bssid");
    if (bssid) {
        if (!cJSON_IsString(bssid) || !parse_mac(bssid->valuestring, out->bssid)) {
            return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
        }
        out->bssid_set = true;
    }

    err = copy_optional_u8(json, "channel", &out->channel, &out->channel_set, 1, 14);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    err = copy_optional_bool(json, "passive", &out->passive);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        return err;
    }

    return copy_optional_u32(json, "timeout_ms", &out->timeout_ms);
}

cJSON* pres_http_dto_wifiman_status_to_json(const dom_usecases_wifiman_status_t* status) {
    if (!status) {
        return NULL;
    }

    cJSON* root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    cJSON_AddItemToObject(root, "wifi", wifi_status_to_json(&status->wifi));
    cJSON_AddBoolToObject(root, "sta_netif_available", status->sta_netif_available);
    if (status->sta_netif_available) {
        cJSON_AddItemToObject(root, "sta_netif", network_interface_to_json(&status->sta_netif));
    }
    cJSON_AddItemToObject(root, "stored_sta", pres_http_dto_wifiman_stored_sta_to_json(&status->stored_sta));
    cJSON_AddBoolToObject(root, "auto_reconnect_enabled", status->auto_reconnect_enabled);
    cJSON_AddNumberToObject(root, "reconnect_trial_count", (double)status->reconnect_trial_count);
    cJSON_AddNumberToObject(root, "reconnect_max_trials", (double)status->reconnect_max_trials);

    return root;
}

cJSON* pres_http_dto_wifiman_scan_result_to_json(const dom_models_wifi_scan_result_t* result) {
    if (!result) {
        return NULL;
    }

    cJSON* root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, "status", wifi_scan_status_to_string(result->status));
    cJSON_AddNumberToObject(root, "status_code", (double)result->status);
    cJSON_AddNumberToObject(root, "scan_id", (double)result->scan_id);
    cJSON_AddNumberToObject(root, "driver_status", (double)result->driver_status);
    cJSON_AddNumberToObject(root, "total_count", (double)result->total_count);
    cJSON_AddNumberToObject(root, "count", (double)result->count);
    cJSON_AddBoolToObject(root, "truncated", result->truncated);

    cJSON* records = cJSON_AddArrayToObject(root, "records");
    if (records) {
        for (size_t i = 0; i < result->count; i++) {
            cJSON_AddItemToArray(records, ap_record_to_json(&result->records[i]));
        }
    }

    return root;
}

cJSON* pres_http_dto_wifiman_stored_sta_to_json(const dom_usecases_wifiman_stored_sta_t* stored_sta) {
    if (!stored_sta) {
        return NULL;
    }

    cJSON* root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    cJSON_AddBoolToObject(root, "available", stored_sta->available);
    if (stored_sta->available) {
        cJSON_AddStringToObject(root, "ssid", stored_sta->ssid);
    }

    return root;
}

cJSON* pres_http_dto_wifiman_reconnect_need_to_json(bool needed) {
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    cJSON_AddBoolToObject(root, "needed", needed);

    return root;
}

cJSON* pres_http_dto_wifiman_reconnect_attempted_to_json(bool attempted) {
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    cJSON_AddBoolToObject(root, "attempted", attempted);

    return root;
}

cJSON* pres_http_dto_wifiman_accepted_to_json(void) {
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    cJSON_AddBoolToObject(root, "accepted", true);

    return root;
}

cJSON* pres_http_dto_wifiman_forgotten_to_json(void) {
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    cJSON_AddBoolToObject(root, "forgotten", true);

    return root;
}

/* Helper Function Implementations */

static size_t bounded_strlen(const char* value, size_t max_len) {
    size_t len = 0;

    if (!value) {
        return 0;
    }

    while (len < max_len && value[len] != '\0') {
        len++;
    }

    return len;
}

static dom_models_error_t copy_json_string(cJSON* json, const char* key, char* out, size_t out_size, bool required) {
    if (!json || !key || !out || out_size == 0) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    cJSON* value = cJSON_GetObjectItemCaseSensitive(json, key);
    if (!value) {
        if (required) {
            return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
        }

        out[0] = '\0';
        return DOMAIN_MODELS_ERROR_OK;
    }
    if (!cJSON_IsString(value) || !value->valuestring) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    size_t len = bounded_strlen(value->valuestring, out_size);
    if (len >= out_size) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    memcpy(out, value->valuestring, len);
    out[len] = '\0';

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t copy_optional_bool(cJSON* json, const char* key, bool* out) {
    cJSON* value = cJSON_GetObjectItemCaseSensitive(json, key);
    if (!value) {
        return DOMAIN_MODELS_ERROR_OK;
    }
    if (!cJSON_IsBool(value)) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    *out = cJSON_IsTrue(value);

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t copy_optional_u8(cJSON* json, const char* key, uint8_t* out, bool* set, uint8_t min, uint8_t max) {
    cJSON* value = cJSON_GetObjectItemCaseSensitive(json, key);
    if (!value) {
        return DOMAIN_MODELS_ERROR_OK;
    }
    if (!cJSON_IsNumber(value) || value->valueint < min || value->valueint > max) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    *out = (uint8_t)value->valueint;
    *set = true;

    return DOMAIN_MODELS_ERROR_OK;
}

static dom_models_error_t copy_optional_u32(cJSON* json, const char* key, uint32_t* out) {
    cJSON* value = cJSON_GetObjectItemCaseSensitive(json, key);
    if (!value) {
        return DOMAIN_MODELS_ERROR_OK;
    }
    if (!cJSON_IsNumber(value) || value->valuedouble < 0.0 || value->valuedouble > 4294967295.0) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    *out = (uint32_t)value->valuedouble;

    return DOMAIN_MODELS_ERROR_OK;
}

static bool parse_mac(const char* value, uint8_t out[DOM_MODELS_WIFI_MAC_LEN]) {
    unsigned int bytes[DOM_MODELS_WIFI_MAC_LEN];

    if (!value || !out) {
        return false;
    }
    if (bounded_strlen(value, 18) != 17) {
        return false;
    }
    if (value[2] != ':' || value[5] != ':' || value[8] != ':' || value[11] != ':' || value[14] != ':') {
        return false;
    }

    if (sscanf(
            value,
            "%2x:%2x:%2x:%2x:%2x:%2x",
            &bytes[0],
            &bytes[1],
            &bytes[2],
            &bytes[3],
            &bytes[4],
            &bytes[5]
        ) != DOM_MODELS_WIFI_MAC_LEN) {
        return false;
    }

    for (size_t i = 0; i < DOM_MODELS_WIFI_MAC_LEN; i++) {
        if (bytes[i] > 0xFF) {
            return false;
        }
        out[i] = (uint8_t)bytes[i];
    }

    return true;
}

static void mac_to_string(const uint8_t mac[DOM_MODELS_WIFI_MAC_LEN], char out[18]) {
    snprintf(
        out,
        18,
        "%02X:%02X:%02X:%02X:%02X:%02X",
        mac[0],
        mac[1],
        mac[2],
        mac[3],
        mac[4],
        mac[5]
    );
}

static void ipv4_to_string(const uint8_t value[DOM_MODELS_NETWORK_IPV4_LEN], char out[16]) {
    snprintf(out, 16, "%u.%u.%u.%u", value[0], value[1], value[2], value[3]);
}

static void ipv6_to_string(const uint8_t value[DOM_MODELS_NETWORK_IPV6_LEN], char out[40]) {
    snprintf(
        out,
        40,
        "%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X",
        value[0],
        value[1],
        value[2],
        value[3],
        value[4],
        value[5],
        value[6],
        value[7],
        value[8],
        value[9],
        value[10],
        value[11],
        value[12],
        value[13],
        value[14],
        value[15]
    );
}

static const char* wifi_mode_to_string(dom_models_wifi_mode_t mode) {
    switch (mode) {
        case DOM_MODELS_WIFI_MODE_NULL:
            return "NULL";
        case DOM_MODELS_WIFI_MODE_STA:
            return "STA";
        case DOM_MODELS_WIFI_MODE_AP:
            return "AP";
        case DOM_MODELS_WIFI_MODE_APSTA:
            return "APSTA";
        case DOM_MODELS_WIFI_MODE_NAN:
            return "NAN";
        case DOM_MODELS_WIFI_MODE_OTHER:
        default:
            return "OTHER";
    }
}

static const char* wifi_auth_to_string(dom_models_wifi_auth_mode_t auth_mode) {
    switch (auth_mode) {
        case DOM_MODELS_WIFI_AUTH_OPEN:
            return "OPEN";
        case DOM_MODELS_WIFI_AUTH_WEP:
            return "WEP";
        case DOM_MODELS_WIFI_AUTH_WPA_PSK:
            return "WPA_PSK";
        case DOM_MODELS_WIFI_AUTH_WPA2_PSK:
            return "WPA2_PSK";
        case DOM_MODELS_WIFI_AUTH_WPA_WPA2_PSK:
            return "WPA_WPA2_PSK";
        case DOM_MODELS_WIFI_AUTH_WPA_ENTERPRISE:
            return "WPA_ENTERPRISE";
        case DOM_MODELS_WIFI_AUTH_WPA2_ENTERPRISE:
            return "WPA2_ENTERPRISE";
        case DOM_MODELS_WIFI_AUTH_WPA3_PSK:
            return "WPA3_PSK";
        case DOM_MODELS_WIFI_AUTH_WPA2_WPA3_PSK:
            return "WPA2_WPA3_PSK";
        case DOM_MODELS_WIFI_AUTH_WAPI_PSK:
            return "WAPI_PSK";
        case DOM_MODELS_WIFI_AUTH_OWE:
            return "OWE";
        case DOM_MODELS_WIFI_AUTH_WPA3_ENT_192:
            return "WPA3_ENT_192";
        case DOM_MODELS_WIFI_AUTH_DPP:
            return "DPP";
        case DOM_MODELS_WIFI_AUTH_WPA3_ENTERPRISE:
            return "WPA3_ENTERPRISE";
        case DOM_MODELS_WIFI_AUTH_WPA2_WPA3_ENTERPRISE:
            return "WPA2_WPA3_ENTERPRISE";
        case DOM_MODELS_WIFI_AUTH_UNKNOWN:
            return "UNKNOWN";
        case DOM_MODELS_WIFI_AUTH_OTHER:
        default:
            return "OTHER";
    }
}

static const char* wifi_cipher_to_string(dom_models_wifi_cipher_t cipher) {
    switch (cipher) {
        case DOM_MODELS_WIFI_CIPHER_NONE:
            return "NONE";
        case DOM_MODELS_WIFI_CIPHER_WEP40:
            return "WEP40";
        case DOM_MODELS_WIFI_CIPHER_WEP104:
            return "WEP104";
        case DOM_MODELS_WIFI_CIPHER_TKIP:
            return "TKIP";
        case DOM_MODELS_WIFI_CIPHER_CCMP:
            return "CCMP";
        case DOM_MODELS_WIFI_CIPHER_TKIP_CCMP:
            return "TKIP_CCMP";
        case DOM_MODELS_WIFI_CIPHER_AES_CMAC128:
            return "AES_CMAC128";
        case DOM_MODELS_WIFI_CIPHER_SMS4:
            return "SMS4";
        case DOM_MODELS_WIFI_CIPHER_GCMP:
            return "GCMP";
        case DOM_MODELS_WIFI_CIPHER_GCMP256:
            return "GCMP256";
        case DOM_MODELS_WIFI_CIPHER_AES_GMAC128:
            return "AES_GMAC128";
        case DOM_MODELS_WIFI_CIPHER_AES_GMAC256:
            return "AES_GMAC256";
        case DOM_MODELS_WIFI_CIPHER_UNKNOWN:
            return "UNKNOWN";
        case DOM_MODELS_WIFI_CIPHER_OTHER:
        default:
            return "OTHER";
    }
}

static const char* wifi_bandwidth_to_string(dom_models_wifi_bandwidth_t bandwidth) {
    switch (bandwidth) {
        case DOM_MODELS_WIFI_BANDWIDTH_20MHZ:
            return "20MHZ";
        case DOM_MODELS_WIFI_BANDWIDTH_40MHZ:
            return "40MHZ";
        case DOM_MODELS_WIFI_BANDWIDTH_80MHZ:
            return "80MHZ";
        case DOM_MODELS_WIFI_BANDWIDTH_160MHZ:
            return "160MHZ";
        case DOM_MODELS_WIFI_BANDWIDTH_80_80MHZ:
            return "80_80MHZ";
        case DOM_MODELS_WIFI_BANDWIDTH_UNKNOWN:
            return "UNKNOWN";
        case DOM_MODELS_WIFI_BANDWIDTH_OTHER:
        default:
            return "OTHER";
    }
}

static const char* wifi_second_channel_to_string(dom_models_wifi_second_channel_t second_channel) {
    switch (second_channel) {
        case DOM_MODELS_WIFI_SECOND_CHANNEL_NONE:
            return "NONE";
        case DOM_MODELS_WIFI_SECOND_CHANNEL_ABOVE:
            return "ABOVE";
        case DOM_MODELS_WIFI_SECOND_CHANNEL_BELOW:
            return "BELOW";
        case DOM_MODELS_WIFI_SECOND_CHANNEL_OTHER:
        default:
            return "OTHER";
    }
}

static const char* wifi_scan_status_to_string(dom_models_wifi_scan_status_t status) {
    switch (status) {
        case DOM_MODELS_WIFI_SCAN_STATUS_IDLE:
            return "IDLE";
        case DOM_MODELS_WIFI_SCAN_STATUS_RUNNING:
            return "RUNNING";
        case DOM_MODELS_WIFI_SCAN_STATUS_DONE:
            return "DONE";
        case DOM_MODELS_WIFI_SCAN_STATUS_FAILED:
            return "FAILED";
        default:
            return "UNKNOWN";
    }
}

static const char* network_interface_type_to_string(dom_models_network_interface_type_t type) {
    switch (type) {
        case DOM_MODELS_NETWORK_INTERFACE_TYPE_WIFI_STA:
            return "WIFI_STA";
        case DOM_MODELS_NETWORK_INTERFACE_TYPE_WIFI_AP:
            return "WIFI_AP";
        case DOM_MODELS_NETWORK_INTERFACE_TYPE_ETHERNET:
            return "ETHERNET";
        case DOM_MODELS_NETWORK_INTERFACE_TYPE_PPP:
            return "PPP";
        case DOM_MODELS_NETWORK_INTERFACE_TYPE_BRIDGE:
            return "BRIDGE";
        case DOM_MODELS_NETWORK_INTERFACE_TYPE_LOOPBACK:
            return "LOOPBACK";
        case DOM_MODELS_NETWORK_INTERFACE_TYPE_UNKNOWN:
        default:
            return "UNKNOWN";
    }
}

static const char* network_dhcp_status_to_string(dom_models_network_dhcp_status_t status) {
    switch (status) {
        case DOM_MODELS_NETWORK_DHCP_STATUS_INIT:
            return "INIT";
        case DOM_MODELS_NETWORK_DHCP_STATUS_STARTED:
            return "STARTED";
        case DOM_MODELS_NETWORK_DHCP_STATUS_STOPPED:
            return "STOPPED";
        case DOM_MODELS_NETWORK_DHCP_STATUS_UNKNOWN:
        default:
            return "UNKNOWN";
    }
}

static cJSON* ap_record_to_json(const dom_models_wifi_ap_record_t* record) {
    char mac[18];
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    cJSON_AddBoolToObject(root, "bssid_available", record->bssid_available);
    if (record->bssid_available) {
        mac_to_string(record->bssid, mac);
        cJSON_AddStringToObject(root, "bssid", mac);
    }
    cJSON_AddStringToObject(root, "ssid", record->ssid);
    cJSON_AddNumberToObject(root, "primary_channel", record->primary_channel);
    cJSON_AddStringToObject(root, "second_channel", wifi_second_channel_to_string(record->second_channel));
    cJSON_AddNumberToObject(root, "second_channel_code", (double)record->second_channel);
    cJSON_AddNumberToObject(root, "rssi", record->rssi);
    cJSON_AddStringToObject(root, "auth_mode", wifi_auth_to_string(record->auth_mode));
    cJSON_AddNumberToObject(root, "auth_mode_code", (double)record->auth_mode);
    cJSON_AddStringToObject(root, "pairwise_cipher", wifi_cipher_to_string(record->pairwise_cipher));
    cJSON_AddNumberToObject(root, "pairwise_cipher_code", (double)record->pairwise_cipher);
    cJSON_AddStringToObject(root, "group_cipher", wifi_cipher_to_string(record->group_cipher));
    cJSON_AddNumberToObject(root, "group_cipher_code", (double)record->group_cipher);
    cJSON_AddStringToObject(root, "bandwidth", wifi_bandwidth_to_string(record->bandwidth));
    cJSON_AddNumberToObject(root, "bandwidth_code", (double)record->bandwidth);
    cJSON_AddNumberToObject(root, "phy_flags", (double)record->phy_flags);

    return root;
}

static cJSON* ap_client_to_json(const dom_models_wifi_ap_client_t* client) {
    char mac[18];
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    mac_to_string(client->mac, mac);
    cJSON_AddStringToObject(root, "mac", mac);
    cJSON_AddNumberToObject(root, "rssi", client->rssi);
    cJSON_AddNumberToObject(root, "phy_flags", (double)client->phy_flags);

    return root;
}

static cJSON* wifi_status_to_json(const dom_models_wifi_status_t* status) {
    char mac[18];
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, "mode", wifi_mode_to_string(status->mode));
    cJSON_AddNumberToObject(root, "mode_code", (double)status->mode);
    cJSON_AddBoolToObject(root, "started", status->started);
    cJSON_AddBoolToObject(root, "sta_if_key_available", status->sta_if_key_available);
    if (status->sta_if_key_available) {
        cJSON_AddStringToObject(root, "sta_if_key", status->sta_if_key);
    }
    cJSON_AddBoolToObject(root, "ap_if_key_available", status->ap_if_key_available);
    if (status->ap_if_key_available) {
        cJSON_AddStringToObject(root, "ap_if_key", status->ap_if_key);
    }
    cJSON_AddBoolToObject(root, "sta_mac_available", status->sta_mac_available);
    if (status->sta_mac_available) {
        mac_to_string(status->sta_mac, mac);
        cJSON_AddStringToObject(root, "sta_mac", mac);
    }
    cJSON_AddBoolToObject(root, "ap_mac_available", status->ap_mac_available);
    if (status->ap_mac_available) {
        mac_to_string(status->ap_mac, mac);
        cJSON_AddStringToObject(root, "ap_mac", mac);
    }
    cJSON_AddBoolToObject(root, "connected", status->connected);
    cJSON_AddBoolToObject(root, "connected_ap_available", status->connected_ap_available);
    if (status->connected_ap_available) {
        cJSON_AddItemToObject(root, "connected_ap", ap_record_to_json(&status->connected_ap));
    }
    cJSON_AddNumberToObject(root, "ap_client_total_count", (double)status->ap_client_total_count);
    cJSON_AddNumberToObject(root, "ap_client_count", (double)status->ap_client_count);
    cJSON_AddBoolToObject(root, "ap_clients_truncated", status->ap_clients_truncated);

    cJSON* clients = cJSON_AddArrayToObject(root, "ap_clients");
    if (clients) {
        for (size_t i = 0; i < status->ap_client_count; i++) {
            cJSON_AddItemToArray(clients, ap_client_to_json(&status->ap_clients[i]));
        }
    }

    return root;
}

static cJSON* ipv4_info_to_json(const dom_models_network_ipv4_info_t* info) {
    char value[16];
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    cJSON_AddBoolToObject(root, "available", info->available);
    if (info->available) {
        ipv4_to_string(info->ip, value);
        cJSON_AddStringToObject(root, "ip", value);
        ipv4_to_string(info->netmask, value);
        cJSON_AddStringToObject(root, "netmask", value);
        ipv4_to_string(info->gateway, value);
        cJSON_AddStringToObject(root, "gateway", value);
    }

    return root;
}

static cJSON* ipv6_addr_to_json(const dom_models_network_ipv6_addr_t* info) {
    char value[40];
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    cJSON_AddBoolToObject(root, "available", info->available);
    if (info->available) {
        ipv6_to_string(info->addr, value);
        cJSON_AddStringToObject(root, "addr", value);
        cJSON_AddNumberToObject(root, "zone", info->zone);
    }

    return root;
}

static cJSON* dns_info_to_json(const dom_models_network_dns_info_t* info) {
    char value[40];
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    cJSON_AddBoolToObject(root, "available", info->available);
    if (info->available) {
        cJSON_AddNumberToObject(root, "family", (double)info->addr.family);
        cJSON_AddNumberToObject(root, "zone", info->addr.zone);
        if (info->addr.family == DOM_MODELS_NETWORK_IP_FAMILY_IPV4) {
            ipv4_to_string(info->addr.bytes, value);
            cJSON_AddStringToObject(root, "addr", value);
        } else if (info->addr.family == DOM_MODELS_NETWORK_IP_FAMILY_IPV6) {
            ipv6_to_string(info->addr.bytes, value);
            cJSON_AddStringToObject(root, "addr", value);
        }
    }

    return root;
}

static cJSON* network_interface_to_json(const dom_models_network_interface_t* interface) {
    char mac[18];
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, "if_key", interface->if_key);
    cJSON_AddStringToObject(root, "desc", interface->desc);
    cJSON_AddBoolToObject(root, "hostname_available", interface->hostname_available);
    if (interface->hostname_available) {
        cJSON_AddStringToObject(root, "hostname", interface->hostname);
    }
    cJSON_AddBoolToObject(root, "impl_name_available", interface->impl_name_available);
    if (interface->impl_name_available) {
        cJSON_AddStringToObject(root, "impl_name", interface->impl_name);
    }
    cJSON_AddStringToObject(root, "type", network_interface_type_to_string(interface->type));
    cJSON_AddNumberToObject(root, "type_code", (double)interface->type);
    cJSON_AddNumberToObject(root, "flags", (double)interface->flags);
    cJSON_AddBoolToObject(root, "is_default", interface->is_default);
    cJSON_AddBoolToObject(root, "is_up", interface->is_up);
    cJSON_AddBoolToObject(root, "mac_available", interface->mac_available);
    if (interface->mac_available) {
        mac_to_string(interface->mac, mac);
        cJSON_AddStringToObject(root, "mac", mac);
    }
    cJSON_AddBoolToObject(root, "impl_index_available", interface->impl_index_available);
    if (interface->impl_index_available) {
        cJSON_AddNumberToObject(root, "impl_index", interface->impl_index);
    }
    cJSON_AddBoolToObject(root, "route_prio_available", interface->route_prio_available);
    if (interface->route_prio_available) {
        cJSON_AddNumberToObject(root, "route_prio", interface->route_prio);
    }
    cJSON_AddBoolToObject(root, "mtu_available", interface->mtu_available);
    if (interface->mtu_available) {
        cJSON_AddNumberToObject(root, "mtu", interface->mtu);
    }
    cJSON_AddBoolToObject(root, "dhcp_client_status_available", interface->dhcp_client_status_available);
    if (interface->dhcp_client_status_available) {
        cJSON_AddStringToObject(root, "dhcp_client_status", network_dhcp_status_to_string(interface->dhcp_client_status));
    }
    cJSON_AddBoolToObject(root, "dhcp_server_status_available", interface->dhcp_server_status_available);
    if (interface->dhcp_server_status_available) {
        cJSON_AddStringToObject(root, "dhcp_server_status", network_dhcp_status_to_string(interface->dhcp_server_status));
    }
    cJSON_AddItemToObject(root, "ipv4", ipv4_info_to_json(&interface->ipv4));
    cJSON_AddItemToObject(root, "old_ipv4", ipv4_info_to_json(&interface->old_ipv4));
    cJSON_AddItemToObject(root, "ipv6_linklocal", ipv6_addr_to_json(&interface->ipv6_linklocal));
    cJSON_AddItemToObject(root, "ipv6_global", ipv6_addr_to_json(&interface->ipv6_global));

    cJSON* dns = cJSON_AddArrayToObject(root, "dns");
    if (dns) {
        for (size_t i = 0; i < DOM_MODELS_NETWORK_DNS_MAX; i++) {
            cJSON_AddItemToArray(dns, dns_info_to_json(&interface->dns[i]));
        }
    }

    return root;
}
