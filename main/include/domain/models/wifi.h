#ifndef DOMAIN_MODELS_WIFI_H
#define DOMAIN_MODELS_WIFI_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "domain/models/network.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DOM_MODELS_WIFI_MAC_LEN                6
#define DOM_MODELS_WIFI_SSID_MAX_LEN           32
#define DOM_MODELS_WIFI_SSID_BUF_LEN           (DOM_MODELS_WIFI_SSID_MAX_LEN + 1)
#define DOM_MODELS_WIFI_PASSWORD_MAX_LEN       64
#define DOM_MODELS_WIFI_PASSWORD_BUF_LEN       (DOM_MODELS_WIFI_PASSWORD_MAX_LEN + 1)
#define DOM_MODELS_WIFI_SSID_LEN               DOM_MODELS_WIFI_SSID_BUF_LEN
#define DOM_MODELS_WIFI_PASSWORD_LEN           DOM_MODELS_WIFI_PASSWORD_BUF_LEN
#define DOM_MODELS_WIFI_SCAN_RESULT_MAX        16
#define DOM_MODELS_WIFI_AP_CLIENT_MAX          8
#define DOM_MODELS_WIFI_AP_DEFAULT_CHANNEL     1
#define DOM_MODELS_WIFI_AP_DEFAULT_MAX_CLIENTS 4
#define DOM_MODELS_WIFI_AP_DEFAULT_AUTH_MODE   DOM_MODELS_WIFI_AUTH_WPA2_PSK

#define DOM_MODELS_WIFI_PHY_FLAG_11B           (1u << 0)
#define DOM_MODELS_WIFI_PHY_FLAG_11G           (1u << 1)
#define DOM_MODELS_WIFI_PHY_FLAG_11N           (1u << 2)
#define DOM_MODELS_WIFI_PHY_FLAG_LR            (1u << 3)
#define DOM_MODELS_WIFI_PHY_FLAG_11A           (1u << 4)
#define DOM_MODELS_WIFI_PHY_FLAG_11AC          (1u << 5)
#define DOM_MODELS_WIFI_PHY_FLAG_11AX          (1u << 6)
#define DOM_MODELS_WIFI_PHY_FLAG_WPS           (1u << 7)
#define DOM_MODELS_WIFI_PHY_FLAG_FTM_RESPONDER (1u << 8)
#define DOM_MODELS_WIFI_PHY_FLAG_FTM_INITIATOR (1u << 9)

#define DOM_MODELS_WIFI_STA_CREDENTIAL_SSID_KEY "wifi_sta_ssid"
#define DOM_MODELS_WIFI_STA_CREDENTIAL_PASS_KEY "wifi_sta_pass"

typedef enum {
    DOM_MODELS_WIFI_MODE_NULL = 0,
    DOM_MODELS_WIFI_MODE_STA,
    DOM_MODELS_WIFI_MODE_AP,
    DOM_MODELS_WIFI_MODE_APSTA,
    DOM_MODELS_WIFI_MODE_NAN,
    DOM_MODELS_WIFI_MODE_OTHER,
} dom_models_wifi_mode_t;

typedef enum {
    DOM_MODELS_WIFI_AUTH_UNKNOWN = 0,
    DOM_MODELS_WIFI_AUTH_OPEN,
    DOM_MODELS_WIFI_AUTH_WEP,
    DOM_MODELS_WIFI_AUTH_WPA_PSK,
    DOM_MODELS_WIFI_AUTH_WPA2_PSK,
    DOM_MODELS_WIFI_AUTH_WPA_WPA2_PSK,
    DOM_MODELS_WIFI_AUTH_WPA_ENTERPRISE,
    DOM_MODELS_WIFI_AUTH_WPA2_ENTERPRISE,
    DOM_MODELS_WIFI_AUTH_WPA3_PSK,
    DOM_MODELS_WIFI_AUTH_WPA2_WPA3_PSK,
    DOM_MODELS_WIFI_AUTH_WAPI_PSK,
    DOM_MODELS_WIFI_AUTH_OWE,
    DOM_MODELS_WIFI_AUTH_WPA3_ENT_192,
    DOM_MODELS_WIFI_AUTH_DPP,
    DOM_MODELS_WIFI_AUTH_WPA3_ENTERPRISE,
    DOM_MODELS_WIFI_AUTH_WPA2_WPA3_ENTERPRISE,
    DOM_MODELS_WIFI_AUTH_OTHER,
} dom_models_wifi_auth_mode_t;

typedef enum {
    DOM_MODELS_WIFI_CIPHER_UNKNOWN = 0,
    DOM_MODELS_WIFI_CIPHER_NONE,
    DOM_MODELS_WIFI_CIPHER_WEP40,
    DOM_MODELS_WIFI_CIPHER_WEP104,
    DOM_MODELS_WIFI_CIPHER_TKIP,
    DOM_MODELS_WIFI_CIPHER_CCMP,
    DOM_MODELS_WIFI_CIPHER_TKIP_CCMP,
    DOM_MODELS_WIFI_CIPHER_AES_CMAC128,
    DOM_MODELS_WIFI_CIPHER_SMS4,
    DOM_MODELS_WIFI_CIPHER_GCMP,
    DOM_MODELS_WIFI_CIPHER_GCMP256,
    DOM_MODELS_WIFI_CIPHER_AES_GMAC128,
    DOM_MODELS_WIFI_CIPHER_AES_GMAC256,
    DOM_MODELS_WIFI_CIPHER_OTHER,
} dom_models_wifi_cipher_t;

typedef enum {
    DOM_MODELS_WIFI_BANDWIDTH_UNKNOWN = 0,
    DOM_MODELS_WIFI_BANDWIDTH_20MHZ,
    DOM_MODELS_WIFI_BANDWIDTH_40MHZ,
    DOM_MODELS_WIFI_BANDWIDTH_80MHZ,
    DOM_MODELS_WIFI_BANDWIDTH_160MHZ,
    DOM_MODELS_WIFI_BANDWIDTH_80_80MHZ,
    DOM_MODELS_WIFI_BANDWIDTH_OTHER,
} dom_models_wifi_bandwidth_t;

typedef enum {
    DOM_MODELS_WIFI_SECOND_CHANNEL_NONE = 0,
    DOM_MODELS_WIFI_SECOND_CHANNEL_ABOVE,
    DOM_MODELS_WIFI_SECOND_CHANNEL_BELOW,
    DOM_MODELS_WIFI_SECOND_CHANNEL_OTHER,
} dom_models_wifi_second_channel_t;

typedef struct {
    char ssid[DOM_MODELS_WIFI_SSID_BUF_LEN];
    char password[DOM_MODELS_WIFI_PASSWORD_BUF_LEN];
} dom_models_wifi_sta_credential_t;

typedef struct {
    bool                             bssid_available;
    uint8_t                          bssid[DOM_MODELS_WIFI_MAC_LEN];
    char                             ssid[DOM_MODELS_WIFI_SSID_BUF_LEN];
    uint8_t                          primary_channel;
    dom_models_wifi_second_channel_t second_channel;
    int8_t                           rssi;
    dom_models_wifi_auth_mode_t      auth_mode;
    dom_models_wifi_cipher_t         pairwise_cipher;
    dom_models_wifi_cipher_t         group_cipher;
    dom_models_wifi_bandwidth_t      bandwidth;
    uint32_t                         phy_flags;
} dom_models_wifi_ap_record_t;

typedef struct {
    uint8_t  mac[DOM_MODELS_WIFI_MAC_LEN];
    int8_t   rssi;
    uint32_t phy_flags;
} dom_models_wifi_ap_client_t;

typedef struct {
    char ssid[DOM_MODELS_WIFI_SSID_BUF_LEN];
    char password[DOM_MODELS_WIFI_PASSWORD_BUF_LEN];

    bool    bssid_set;
    uint8_t bssid[DOM_MODELS_WIFI_MAC_LEN];

    bool    channel_set;
    uint8_t channel;
} dom_models_wifi_sta_connect_config_t;

typedef struct {
    char ssid[DOM_MODELS_WIFI_SSID_BUF_LEN];
    char password[DOM_MODELS_WIFI_PASSWORD_BUF_LEN];
    bool hidden;

    bool    channel_set;
    uint8_t channel;

    bool                        auth_mode_set;
    dom_models_wifi_auth_mode_t auth_mode;

    bool    max_clients_set;
    uint8_t max_clients;
} dom_models_wifi_ap_config_t;

typedef struct {
    bool ssid_set;
    char ssid[DOM_MODELS_WIFI_SSID_BUF_LEN];

    bool    bssid_set;
    uint8_t bssid[DOM_MODELS_WIFI_MAC_LEN];

    bool    channel_set;
    uint8_t channel;

    bool     passive;
    uint32_t timeout_ms;
} dom_models_wifi_scan_config_t;

typedef enum {
    DOM_MODELS_WIFI_SCAN_STATUS_IDLE = 0,
    DOM_MODELS_WIFI_SCAN_STATUS_RUNNING,
    DOM_MODELS_WIFI_SCAN_STATUS_DONE,
    DOM_MODELS_WIFI_SCAN_STATUS_FAILED,
} dom_models_wifi_scan_status_t;

typedef struct {
    dom_models_wifi_scan_status_t status;
    uint32_t                      scan_id;
    uint32_t                      driver_status;
    size_t                        total_count;
    size_t                        count;
    bool                          truncated;
    dom_models_wifi_ap_record_t   records[DOM_MODELS_WIFI_SCAN_RESULT_MAX];
} dom_models_wifi_scan_result_t;

typedef struct {
    dom_models_wifi_mode_t mode;
    bool                   started;

    bool sta_if_key_available;
    char sta_if_key[DOM_MODELS_NETWORK_IF_KEY_LEN];
    bool ap_if_key_available;
    char ap_if_key[DOM_MODELS_NETWORK_IF_KEY_LEN];

    bool    sta_mac_available;
    uint8_t sta_mac[DOM_MODELS_WIFI_MAC_LEN];
    bool    ap_mac_available;
    uint8_t ap_mac[DOM_MODELS_WIFI_MAC_LEN];

    bool                        connected;
    bool                        connected_ap_available;
    dom_models_wifi_ap_record_t connected_ap;

    size_t                      ap_client_total_count;
    size_t                      ap_client_count;
    bool                        ap_clients_truncated;
    dom_models_wifi_ap_client_t ap_clients[DOM_MODELS_WIFI_AP_CLIENT_MAX];
} dom_models_wifi_status_t;

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_MODELS_WIFI_H */
