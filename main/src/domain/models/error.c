#include "domain/models/error.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERROR_DEFAULT_ACTOR   "unknown"
#define ERROR_ACTOR_PREFIX    "["
#define ERROR_ACTOR_SUFFIX    "] "
#define ERROR_TYPE_SEPARATOR  ": "
#define ERROR_CHAIN_SEPARATOR " <- "

/* Helper Function Prototypes */

static size_t              add_length(size_t base, size_t extra);
static size_t              actor_length(const dom_models_error_t* handle);
static void                append_to_buffer(char* buf, size_t buf_size, size_t* len, const char* value, size_t value_len);
static void                copy_actor(dom_models_error_t* handle, const char* actor);
static void                format_message(dom_models_error_t* handle, const char* fmt, va_list args);
static dom_models_error_t* error_create(dom_models_error_t* prev, const char* actor, dom_models_error_type_t type, const char* fmt, va_list args);

/* Implementations */

dom_models_error_t* dom_models_error_new(const char* actor, dom_models_error_type_t type, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    dom_models_error_t* handle = error_create(NULL, actor, type, fmt, args);
    va_end(args);

    return handle;
}

void dom_models_error_delete(dom_models_error_t* handle) {
    if (!handle) {
        return;
    }

    while (handle) {
        dom_models_error_t* prev = handle->prev;
        free(handle->msg);
        free(handle);
        handle = prev;
    }
}

dom_models_error_t* dom_models_error_append(dom_models_error_t* handle, const char* actor, dom_models_error_type_t type, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    dom_models_error_t* next = error_create(handle, actor, type, fmt, args);
    va_end(args);

    if (!next) {
        return handle;
    }

    return next;
}

size_t dom_models_error_total_length(const dom_models_error_t* handle) {
    size_t len = 0;

    for (const dom_models_error_t* cursor = handle; cursor; cursor = cursor->prev) {
        const char* type = dom_models_error_type_str(cursor->type);
        size_t      actor_len = actor_length(cursor);

        if (cursor != handle) {
            len = add_length(len, sizeof(ERROR_CHAIN_SEPARATOR) - 1);
        }

        len = add_length(len, sizeof(ERROR_ACTOR_PREFIX) - 1);
        len = add_length(len, actor_len);
        len = add_length(len, sizeof(ERROR_ACTOR_SUFFIX) - 1);
        len = add_length(len, strlen(type));
        if (cursor->msg && cursor->len > 0) {
            len = add_length(len, sizeof(ERROR_TYPE_SEPARATOR) - 1);
            len = add_length(len, cursor->len);
        }
    }

    return len;
}

size_t dom_models_error_print(const dom_models_error_t* handle, char* buf, size_t buf_size) {
    size_t len = 0;

    for (const dom_models_error_t* cursor = handle; cursor; cursor = cursor->prev) {
        const char* type = dom_models_error_type_str(cursor->type);
        size_t      actor_len = actor_length(cursor);

        if (cursor != handle) {
            append_to_buffer(buf, buf_size, &len, ERROR_CHAIN_SEPARATOR, sizeof(ERROR_CHAIN_SEPARATOR) - 1);
        }

        append_to_buffer(buf, buf_size, &len, ERROR_ACTOR_PREFIX, sizeof(ERROR_ACTOR_PREFIX) - 1);
        append_to_buffer(buf, buf_size, &len, cursor->actor, actor_len);
        append_to_buffer(buf, buf_size, &len, ERROR_ACTOR_SUFFIX, sizeof(ERROR_ACTOR_SUFFIX) - 1);
        append_to_buffer(buf, buf_size, &len, type, strlen(type));
        if (cursor->msg && cursor->len > 0) {
            append_to_buffer(buf, buf_size, &len, ERROR_TYPE_SEPARATOR, sizeof(ERROR_TYPE_SEPARATOR) - 1);
            append_to_buffer(buf, buf_size, &len, cursor->msg, cursor->len);
        }
    }

    if (buf && buf_size > 0) {
        size_t end = len < buf_size ? len : buf_size - 1;
        buf[end]   = '\0';
    }

    return len;
}

/* Helper Function Implementations */

static size_t add_length(size_t base, size_t extra) {
    if (SIZE_MAX - base < extra) {
        return SIZE_MAX;
    }
    return base + extra;
}

static size_t actor_length(const dom_models_error_t* handle) {
    size_t len = 0;
    while (len < sizeof(handle->actor) && handle->actor[len] != '\0') {
        len++;
    }
    return len;
}

static void append_to_buffer(char* buf, size_t buf_size, size_t* len, const char* value, size_t value_len) {
    if (!value || value_len == 0) {
        return;
    }

    if (buf && buf_size > 0 && *len < buf_size - 1) {
        size_t available = (buf_size - 1) - *len;
        size_t copy_len  = value_len < available ? value_len : available;
        memcpy(buf + *len, value, copy_len);
    }

    *len = add_length(*len, value_len);
}

static void copy_actor(dom_models_error_t* handle, const char* actor) {
    const char* value = actor && actor[0] != '\0' ? actor : ERROR_DEFAULT_ACTOR;
    snprintf(handle->actor, sizeof(handle->actor), "%s", value);
}

static void format_message(dom_models_error_t* handle, const char* fmt, va_list args) {
    if (!fmt) {
        return;
    }

    va_list args_copy;
    va_copy(args_copy, args);
    int len = vsnprintf(NULL, 0, fmt, args_copy);
    va_end(args_copy);

    if (len <= 0) {
        return;
    }

    handle->msg = (char*)malloc((size_t)len + 1);
    if (!handle->msg) {
        return;
    }

    int written = vsnprintf(handle->msg, (size_t)len + 1, fmt, args);
    if (written < 0) {
        free(handle->msg);
        handle->msg = NULL;
        return;
    }

    handle->len = (size_t)written < (size_t)len ? (size_t)written : (size_t)len;
}

static dom_models_error_t* error_create(dom_models_error_t* prev, const char* actor, dom_models_error_type_t type, const char* fmt, va_list args) {
    dom_models_error_t* handle = (dom_models_error_t*)malloc(sizeof(dom_models_error_t));
    if (!handle) {
        return NULL;
    }

    handle->msg  = NULL;
    handle->len  = 0;
    handle->type = type;
    handle->prev = prev;

    copy_actor(handle, actor);
    format_message(handle, fmt, args);

    return handle;
}
