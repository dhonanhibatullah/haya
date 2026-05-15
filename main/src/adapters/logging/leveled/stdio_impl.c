#include "adapters/logging/leveled/stdio_impl.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "domain/models/log.h"
#include "domain/ports/outbound/logging/leveled.h"

// Helpers

static void print_log(
    const char* level,
    const char* tag,
    const char* format,
    va_list     args
) {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    struct tm timeinfo;
    localtime_r(&tv.tv_sec, &timeinfo);

    printf(
        "%02d/%02d/%04d %02d:%02d:%02d.%03ld [%s] [%s] ",
        timeinfo.tm_mday,
        timeinfo.tm_mon + 1,
        timeinfo.tm_year + 1900,
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec,
        tv.tv_usec / 1000,
        level,
        tag
    );
    vprintf(format, args);
    printf("\n");
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

    adp_logging_leveled_stdio_impl_ctx_t* ctx = self->ctx;
    if (ctx->cfg.level < DOMAIN_MODELS_LOG_LEVEL_ERROR) {
        return;
    }

    va_list args;
    va_start(args, format);
    print_log("ERROR", tag, format, args);
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

    adp_logging_leveled_stdio_impl_ctx_t* ctx = self->ctx;
    if (ctx->cfg.level < DOMAIN_MODELS_LOG_LEVEL_WARN) {
        return;
    }

    va_list args;
    va_start(args, format);
    print_log("WARN", tag, format, args);
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

    adp_logging_leveled_stdio_impl_ctx_t* ctx = self->ctx;
    if (ctx->cfg.level < DOMAIN_MODELS_LOG_LEVEL_INFO) {
        return;
    }

    va_list args;
    va_start(args, format);
    print_log("INFO", tag, format, args);
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

    adp_logging_leveled_stdio_impl_ctx_t* ctx = self->ctx;
    if (ctx->cfg.level < DOMAIN_MODELS_LOG_LEVEL_DEBUG) {
        return;
    }

    va_list args;
    va_start(args, format);
    print_log("DEBUG", tag, format, args);
    va_end(args);
}

// Constructor & destructor

dpo_logging_leveled_t* adp_logging_leveled_stdio_impl_new(const adp_logging_leveled_stdio_impl_cfg_t* cfg) {
    adp_logging_leveled_stdio_impl_ctx_t* ctx = calloc(1, sizeof(adp_logging_leveled_stdio_impl_ctx_t));
    if (!ctx) {
        return NULL;
    }
    if (!cfg) {
        memcpy(&ctx->cfg, cfg, sizeof(adp_logging_leveled_stdio_impl_ctx_t));
    } else {
        ctx->cfg.level = DOMAIN_MODELS_LOG_LEVEL_INFO;
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

void adp_logging_leveled_stdio_impl_delete(dpo_logging_leveled_t* self) {
    if (!self) {
        return;
    }
    free(self->ctx);
    dpo_logging_leveled_delete(self);
}
