#include "presentation/mqtt/handler/ota.h"

#include <stdlib.h>
#include <string.h>

#include "cJSON.h"
#include "domain/models/update.h"
#include "freertos/FreeRTOS.h"  // IWYU pragma: keep
#include "freertos/task.h"

#define TAG "pres_mqtt_ota"

typedef struct {
    pres_mqtt_context_t*     ctx;
    dom_models_update_info_t update_info;
} ota_task_args_t;

/* Update Task Prototype */

static void ota_update_task(void* pvParameters);

/* Handler Implementation */

void pres_mqtt_handler_ota(pres_mqtt_context_t* ctx, const char* data, int data_len) {
    ctx->logger->info(ctx->logger, TAG, "Received OTA update trigger via MQTT");

    if (!data || data_len <= 0) {
        ctx->logger->error(ctx->logger, TAG, "Empty payload received");
        return;
    }

    cJSON* json = cJSON_ParseWithLength(data, data_len);
    if (!json) {
        ctx->logger->error(ctx->logger, TAG, "Failed to parse JSON payload");
        return;
    }

    cJSON* url_item      = cJSON_GetObjectItemCaseSensitive(json, "url");
    cJSON* size_item     = cJSON_GetObjectItemCaseSensitive(json, "size");
    cJSON* checksum_item = cJSON_GetObjectItemCaseSensitive(json, "checksum");

    if (!cJSON_IsString(url_item) || !cJSON_IsNumber(size_item) || !cJSON_IsString(checksum_item)) {
        ctx->logger->error(ctx->logger, TAG, "Invalid OTA payload fields");
        cJSON_Delete(json);
        return;
    }

    ota_task_args_t* task_args = (ota_task_args_t*)calloc(1, sizeof(ota_task_args_t));
    if (!task_args) {
        ctx->logger->error(ctx->logger, TAG, "Failed to allocate memory for task args");
        cJSON_Delete(json);
        return;
    }

    task_args->ctx = ctx;
    strncpy(task_args->update_info.firmware_url, url_item->valuestring, sizeof(task_args->update_info.firmware_url) - 1);
    task_args->update_info.firmware_size = (size_t)size_item->valuedouble;
    strncpy(task_args->update_info.firmware_checksum, checksum_item->valuestring, sizeof(task_args->update_info.firmware_checksum) - 1);

    cJSON_Delete(json);

    ctx->logger->info(ctx->logger, TAG, "Spawning background OTA task for URL: %s", task_args->update_info.firmware_url);

    // Spawn a FreeRTOS task to run the update asynchronously to avoid blocking the MQTT client event loop thread.
    BaseType_t ret = xTaskCreate(
        ota_update_task,
        "pres_mqtt_ota_task",
        8192,
        task_args,
        5,
        NULL
    );

    if (ret != pdPASS) {
        ctx->logger->error(ctx->logger, TAG, "Failed to create background OTA task");
        free(task_args);
    }
}

/* Update Task Implementation */

static void ota_update_task(void* pvParameters) {
    ota_task_args_t*     args = (ota_task_args_t*)pvParameters;
    pres_mqtt_context_t* ctx  = args->ctx;

    ctx->logger->info(ctx->logger, TAG, "OTA background task started");

    dom_models_error_t err = ctx->ota->update(ctx->ota, &args->update_info);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ctx->logger->error(ctx->logger, TAG, "OTA update failed in background: %s (%d)", dom_models_error_str(err), (int)err);
    } else {
        ctx->logger->info(ctx->logger, TAG, "OTA update succeeded, system will reboot");
    }

    free(args);
    vTaskDelete(NULL);
}
