#include "adapters/logging/leveled/esp_impl.h"

#include <stdarg.h>
#include <string.h>

#include "domain/ports/outbound/logging/leveled.h"
#include "esp_log_write.h"

// Adapter implementations

static void error_impl(
    dpo_logging_leveled_t* self,
    const char*            tag,
    const char*            format,
    ...
) {
    (void)self;
    va_list args;
    va_start(args, format);
    esp_log_writev(ESP_LOG_ERROR, tag, format, args);
    va_end(args);
}

static void warn_impl(
    dpo_logging_leveled_t* self,
    const char*            tag,
    const char*            format,
    ...
) {
    (void)self;
    va_list args;
    va_start(args, format);
    esp_log_writev(ESP_LOG_WARN, tag, format, args);
    va_end(args);
}

static void info_impl(
    dpo_logging_leveled_t* self,
    const char*            tag,
    const char*            format,
    ...
) {
    (void)self;
    va_list args;
    va_start(args, format);
    esp_log_writev(ESP_LOG_INFO, tag, format, args);
    va_end(args);
}

static void debug_impl(
    dpo_logging_leveled_t* self,
    const char*            tag,
    const char*            format,
    ...
) {
    (void)self;
    va_list args;
    va_start(args, format);
    esp_log_writev(ESP_LOG_DEBUG, tag, format, args);
    va_end(args);
}

// Constructor & destructor

dpo_logging_leveled_t* adp_logging_leveled_esp_impl_new(const adp_logging_leveled_esp_impl_cfg_t* cfg) {
    adp_logging_leveled_esp_impl_ctx_t* ctx = calloc(1, sizeof(adp_logging_leveled_esp_impl_ctx_t));
    if (!ctx) {
        return NULL;
    }
    if (!cfg) {
        memcpy(&ctx->cfg, cfg, sizeof(adp_logging_leveled_esp_impl_ctx_t));
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

void adp_logging_leveled_esp_impl_delete(dpo_logging_leveled_t* self) {
    if (!self) {
        return;
    }
    free(self->ctx);
    dpo_logging_leveled_delete(self);
}
