#include "adapters/logging/leveled/ble_impl.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "domain/models/log.h"
#include "domain/ports/outbound/logging/leveled.h"
#include "esp_err.h"
#include "host/ble_att.h"
#include "host/ble_gatt.h"
#include "host/ble_hs.h"

// Helpers

static esp_err_t nimble_to_esp_err(int rc) {
    if (rc == 0) {
        return ESP_OK;
    }
    if (rc == BLE_HS_ENOMEM) {
        return ESP_ERR_NO_MEM;
    }
    if (rc == BLE_HS_EINVAL) {
        return ESP_ERR_INVALID_ARG;
    }
    return ESP_FAIL;
}

static void print_log(
    adp_logging_leveled_ble_impl_ctx_t* ctx,
    const char*                         level,
    const char*                         tag,
    const char*                         format,
    va_list                             args
) {
    struct timeval tv;
    struct tm      timeinfo;
    int            offset;

    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &timeinfo);

    offset = snprintf(
        ctx->message,
        sizeof(ctx->message),
        "%02d/%02d/%04d %02d:%02d:%02d.%03ld [%s] [%s] ",
        timeinfo.tm_mday,
        timeinfo.tm_mon + 1,
        timeinfo.tm_year + 1900,
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec,
        tv.tv_usec / 1000,
        level,
        tag ? tag : ""
    );
    if (offset < 0) {
        ctx->message[0]  = '\0';
        ctx->message_len = 0;
        return;
    }

    if ((size_t)offset < sizeof(ctx->message)) {
        vsnprintf(
            ctx->message + offset,
            sizeof(ctx->message) - (size_t)offset,
            format ? format : "",
            args
        );
    }

    ctx->message_len = strnlen(ctx->message, sizeof(ctx->message));
    if (ctx->message_len + 1U < sizeof(ctx->message)) {
        ctx->message[ctx->message_len++] = '\n';
        ctx->message[ctx->message_len]   = '\0';
    }
    if (ctx->message_len == 0) {
        return;
    }

    printf("%.*s", ctx->message_len, ctx->message);

    if (ctx->enabled && ctx->message_chr_hdl != 0) {
        ble_gatts_chr_updated(ctx->message_chr_hdl);
    }
}

// BLE access callbacks

static int toggle_access_callback(
    uint16_t                     conn_handle,
    uint16_t                     attr_handle,
    struct ble_gatt_access_ctxt* ctxt,
    void*                        arg
) {
    (void)conn_handle;
    (void)attr_handle;

    if (!arg || !ctxt) {
        return BLE_ATT_ERR_INVALID_HANDLE;
    }
    if (ctxt->op != BLE_GATT_ACCESS_OP_WRITE_CHR) {
        return BLE_ATT_ERR_REQ_NOT_SUPPORTED;
    }

    adp_logging_leveled_ble_impl_ctx_t* ctx = arg;
    struct os_mbuf*                     om  = ctxt->om;

    uint8_t  value;
    uint16_t len;
    int      rc;

    if (OS_MBUF_PKTLEN(om) != sizeof(value)) {
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }

    rc = ble_hs_mbuf_to_flat(om, &value, sizeof(value), &len);
    if (rc != 0 || len != sizeof(value)) {
        return BLE_ATT_ERR_UNLIKELY;
    }

    ctx->enabled = value != 0;
    return 0;
}

static int message_access_callback(
    uint16_t                     conn_handle,
    uint16_t                     attr_handle,
    struct ble_gatt_access_ctxt* ctxt,
    void*                        arg
) {
    (void)conn_handle;
    (void)attr_handle;

    if (!arg || !ctxt) {
        return BLE_ATT_ERR_INVALID_HANDLE;
    }
    if (ctxt->op != BLE_GATT_ACCESS_OP_READ_CHR) {
        return BLE_ATT_ERR_REQ_NOT_SUPPORTED;
    }

    adp_logging_leveled_ble_impl_ctx_t* ctx = arg;
    struct os_mbuf*                     om  = ctxt->om;

    int rc = os_mbuf_append(om, ctx->message, ctx->message_len);
    return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

// Adapter implementations

static void error_impl(
    dpo_logging_leveled_t* self,
    const char*            tag,
    const char*            format,
    ...
) {
    if (!self || !self->ctx) {
        return;
    }

    adp_logging_leveled_ble_impl_ctx_t* ctx = self->ctx;
    if (ctx->cfg.level < DOMAIN_MODELS_LOG_LEVEL_ERROR) {
        return;
    }

    va_list args;
    va_start(args, format);
    print_log(ctx, "ERROR", tag, format, args);
    va_end(args);
}

static void warn_impl(
    dpo_logging_leveled_t* self,
    const char*            tag,
    const char*            format,
    ...
) {
    if (!self || !self->ctx) {
        return;
    }

    adp_logging_leveled_ble_impl_ctx_t* ctx = self->ctx;
    if (ctx->cfg.level < DOMAIN_MODELS_LOG_LEVEL_WARN) {
        return;
    }

    va_list args;
    va_start(args, format);
    print_log(ctx, "WARN", tag, format, args);
    va_end(args);
}

static void info_impl(
    dpo_logging_leveled_t* self,
    const char*            tag,
    const char*            format,
    ...
) {
    if (!self || !self->ctx) {
        return;
    }

    adp_logging_leveled_ble_impl_ctx_t* ctx = self->ctx;
    if (ctx->cfg.level < DOMAIN_MODELS_LOG_LEVEL_INFO) {
        return;
    }

    va_list args;
    va_start(args, format);
    print_log(ctx, "INFO", tag, format, args);
    va_end(args);
}

static void debug_impl(
    dpo_logging_leveled_t* self,
    const char*            tag,
    const char*            format,
    ...
) {
    if (!self || !self->ctx) {
        return;
    }

    adp_logging_leveled_ble_impl_ctx_t* ctx = self->ctx;
    if (ctx->cfg.level < DOMAIN_MODELS_LOG_LEVEL_DEBUG) {
        return;
    }

    va_list args;
    va_start(args, format);
    print_log(ctx, "DEBUG", tag, format, args);
    va_end(args);
}

// Constructor, destructor, initiator, & deinitiator

dpo_logging_leveled_t* adp_logging_leveled_ble_impl_new(const adp_logging_leveled_ble_impl_cfg_t* cfg) {
    adp_logging_leveled_ble_impl_ctx_t* ctx = calloc(1, sizeof(adp_logging_leveled_ble_impl_ctx_t));
    if (!ctx) {
        return NULL;
    }
    if (cfg) {
        memcpy(&ctx->cfg, cfg, sizeof(adp_logging_leveled_ble_impl_cfg_t));
    }

    dpo_logging_leveled_t* self = dpo_logging_leveled_new(ctx);
    if (!self) {
        free(ctx);
        return NULL;
    }
    self->error = error_impl;
    self->warn  = warn_impl;
    self->info  = info_impl;
    self->debug = debug_impl;

    return self;
}

void adp_logging_leveled_ble_impl_delete(dpo_logging_leveled_t* self) {
    if (!self) {
        return;
    }
    free(self->ctx);
    dpo_logging_leveled_delete(self);
}

esp_err_t adp_logging_leveled_ble_impl_init(dpo_logging_leveled_t* self) {
    if (!self || !self->ctx) {
        return ESP_ERR_INVALID_ARG;
    }

    adp_logging_leveled_ble_impl_ctx_t* ctx = self->ctx;

    ctx->characteristic_defs[0] = (adp_logging_leveled_ble_impl_chr_def_t){
        .uuid       = &ctx->cfg.toggle_characteristic_uuid.u,
        .access_cb  = toggle_access_callback,
        .arg        = ctx,
        .flags      = BLE_GATT_CHR_F_WRITE,
        .val_handle = &ctx->toggle_chr_hdl,
    };
    ctx->characteristic_defs[1] = (adp_logging_leveled_ble_impl_chr_def_t){
        .uuid       = &ctx->cfg.message_characteristic_uuid.u,
        .access_cb  = message_access_callback,
        .arg        = ctx,
        .flags      = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
        .val_handle = &ctx->message_chr_hdl,
    };
    ctx->characteristic_defs[2] = (adp_logging_leveled_ble_impl_chr_def_t){0};

    ctx->service_defs[0] = (adp_logging_leveled_ble_impl_svc_def_t){
        .type            = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid            = &ctx->cfg.service_uuid.u,
        .characteristics = ctx->characteristic_defs,
    };
    ctx->service_defs[1] = (adp_logging_leveled_ble_impl_svc_def_t){0};

    int rc = ble_gatts_count_cfg(ctx->service_defs);
    if (rc != 0) {
        return nimble_to_esp_err(rc);
    }

    rc = ble_gatts_add_svcs(ctx->service_defs);
    if (rc != 0) {
        return nimble_to_esp_err(rc);
    }

    return ESP_OK;
}

void adp_logging_leveled_ble_impl_deinit(dpo_logging_leveled_t* self) {
    if (!self || !self->ctx) {
        return;
    }

    adp_logging_leveled_ble_impl_ctx_t* ctx = self->ctx;

    ctx->enabled         = false;
    ctx->toggle_chr_hdl  = 0;
    ctx->message_chr_hdl = 0;
    ctx->message[0]      = '\0';
    ctx->message_len     = 0;
}