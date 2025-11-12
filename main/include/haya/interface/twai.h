#ifndef __HAYA_INTERFACE_TWAI_H
#define __HAYA_INTERFACE_TWAI_H

#include "esp_err.h"
#include "esp_twai.h"
#include "esp_twai_onchip.h"

esp_err_t hyInterfaceTWAISetup(
    twai_node_handle_t *twai_handle,
    int tx_pin,
    int rx_pin,
    int bitrate,
    uint32_t *filter_id_list,
    uint32_t filter_id_count,
    uint32_t filter_mask,
    uint32_t filter_is_ext);

#endif