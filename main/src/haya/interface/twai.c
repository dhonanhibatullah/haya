#include "haya/interface/twai.h"

esp_err_t hyInterfaceTWAISetup(
    twai_node_handle_t *twai_handle,
    int tx_pin,
    int rx_pin,
    int bitrate,
    uint32_t *filter_id_list,
    uint32_t filter_id_count,
    uint32_t filter_mask,
    uint32_t filter_is_ext)
{
    twai_onchip_node_config_t twai_config = {
        .io_cfg.tx = tx_pin,
        .io_cfg.rx = rx_pin,
        .bit_timing.bitrate = bitrate,
        .tx_queue_depth = 5};

    esp_err_t err = twai_new_node_onchip(&twai_config, twai_handle);
    if (err != ESP_OK)
        return err;

    if (filter_id_list != NULL)
    {
        twai_mask_filter_config_t twai_maskfilt_config = {
            .id_list = filter_id_list,
            .num_of_ids = filter_id_count,
            .mask = filter_mask,
            .is_ext = filter_is_ext,
            .no_classic = 0,
            .no_fd = 0,
            .dual_filter = 0};

        err = twai_node_config_mask_filter(twai_handle, 0, &twai_maskfilt_config);
        if (err != ESP_OK)
            return err;
    }

    return ESP_OK;
}
