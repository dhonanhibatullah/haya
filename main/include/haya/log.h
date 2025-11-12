/**
 * @file haya/log.h
 * @brief A robust logging wrapper for ESP-IDF.
 *
 * This module intercepts the standard ESP-IDF logging system (`esp_log_...`)
 * to provide two additional features:
 * 1.  **File Logging:** The ability to save all logs to the filesystem
 * with automatic file rotation.
 * 2.  **Custom Callback:** The ability to "tee" all log messages to a
 * custom function (e.g., for sending logs over a network or UART).
 *
 * This module is thread-safe.
 *
 * @note You must call hyLogSetup() once at startup before using any
 * other logging functions.
 */

#ifndef __HAYA_LOG_H
#define __HAYA_LOG_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "haya/error.h"

/**
 * @defgroup haya_log_macros Public Logging Macros
 * @brief These macros are simple aliases for the standard ESP-LOG functions.
 *
 * Using these ensures your logs are correctly processed by the haya log wrapper.
 * @{
 */
#define hyLogError ESP_LOGE   /**< @brief Log a message at ESP_LOGE level. */
#define hyLogWarn ESP_LOGW    /**< @brief Log a message at ESP_LOGW level. */
#define hyLogInfo ESP_LOGI    /**< @brief Log a message at ESP_LOGI level. */
#define hyLogDebug ESP_LOGD   /**< @brief Log a message at ESP_LOGD level. */
#define hyLogVerbose ESP_LOGV /**< @brief Log a message at ESP_LOGV level. */
/** @} */

/**
 * @brief Internal define for the maximum log file path length.
 * @internal
 */
#define _HY_LOG_PATH_MAX_LEN 256

/**
 * @brief Function pointer type for a custom log callback.
 *
 * @param arg The user-defined argument provided in hyLogSetCallback().
 * @param format The log message format string (e.g., "Hello %s").
 * @param args The va_list of log arguments.
 */
typedef void (*HyLogCbFunc)(void *arg, const char *format, va_list args);

/**
 * @brief Internal global handle for the logger state.
 * @note This struct holds the entire state of the logger and is
 * protected by a mutex. Do not access its members directly.
 * @internal
 */
typedef struct
{
    SemaphoreHandle_t mtx;      /**< @brief Mutex for thread-safe access to the handle. */
    vprintf_like_t def_vprintf; /**< @brief Stores the original ESP-IDF vprintf function. */
    volatile bool setup;        /**< @brief Flag (true) if hyLogSetup() has been called. */

    /* --- File Saving State --- */
    bool save_en;               /**< @brief Flag (true) if file logging is enabled. */
    char *dir_path;             /**< @brief Path to the log directory (e.g., "/spiffs/logs"). */
    char *bin_path;             /**< @brief Path to the rotation index file (e.g., "/spiffs/logs/.log.bin"). */
    char *log_path_fmt;         /**< @brief Format string for log files (e.g., "/spiffs/logs/%010d.log"). */
    uint32_t file_keep_num;     /**< @brief Max number of log files to keep during rotation. */
    TickType_t rotation_period; /**< @brief Ticks between log file rotations. */
    TickType_t recover_period;  /**< @brief Ticks to wait before retrying a file write after an error. */
    FILE *f;                    /**< @brief The file handle for the *current* open log file. */
    bool is_err;                /**< @brief Flag (true) if a file write error has occurred. */
    TickType_t check_ts;        /**< @brief Timestamp (in ticks) of the last rotation or error check. */

    /* --- Custom Callback State --- */
    void *cb_arg;        /**< @brief User-defined argument for the custom callback. */
    HyLogCbFunc cb_func; /**< @brief The custom callback function pointer. */
} _HyLogHandle;

/**
 * @brief The global internal logger instance.
 * @internal
 */
extern _HyLogHandle _hy_log;

/**
 * @brief Initializes the logging module.
 *
 * This function *must* be called once at application startup.
 * It creates the internal mutex and sets the ESP-IDF vprintf wrapper
 * to intercept all `ESP_LOG...` and `hyLog...` calls.
 *
 * @return HY_ERR_NONE on success.
 * @return HY_ERR_FAILURE if the mutex cannot be created.
 */
HyErr hyLogSetup();

/**
 * @brief Enables saving log output to the filesystem.
 *
 * This function is thread-safe. It can be called any time after hyLogSetup().
 * It allocates memory for path strings, creates the log directory,
 * and initializes the log rotation index file.
 *
 * @param dir_path The absolute path to the log directory (e.g., "/spiffs/logs").
 * @warning This path **must not** end with a trailing '/'.
 * @param file_keep_num The maximum number of log files to keep. When this
 * number is exceeded, the oldest file is deleted.
 * @param file_rotation_period The time, in ticks, between log file rotations.
 * @param recover_period The time, in ticks, to wait before retrying to write
 * to a file after a write error occurs.
 *
 * @return HY_ERR_NONE on success.
 * @return HY_ERR_FAILURE if not setup, directory/file creation fails.
 * @return HY_ERR_BAD_ARGS if arguments are invalid (e.g., NULL path).
 * @return HY_ERR_MALLOC_FAILED if path string allocation fails.
 */
HyErr hyLogSaveEnable(
    const char *dir_path,
    uint32_t file_keep_num,
    TickType_t file_rotation_period,
    TickType_t recover_period);

/**
 * @brief Disables saving log output to the filesystem.
 *
 * This function is thread-safe. It closes any open log file and
 * frees the memory used for path strings.
 */
void hyLogSaveDisable();

/**
 * @brief Sets a custom callback to receive all log messages.
 *
 * This function is thread-safe. It allows "tee-ing" all log output
 * to another function (e.g., to send over UART or a network).
 *
 * @param cb The callback function. Set to NULL to disable the callback.
 * @param arg A user-defined argument that will be passed to the callback.
 */
void hyLogSetCallback(HyLogCbFunc cb, void *arg);

/**
 * @internal
 * @brief Internal vprintf wrapper that intercepts all log calls.
 *
 * This is the core function set by `esp_log_set_vprintf`. It takes the mutex,
 * then dispatches the log message to:
 * 1. The file save handler (_hyLogSaveHandle)
 * 2. The custom callback (cb_func)
 * 3. The original, default vprintf function
 *
 * @param format The log message format string.
 * @param args The va_list of log arguments.
 * @return The return value of the default vprintf function.
 */
int _hyLogWrapper(const char *format, va_list args);

/**
 * @internal
 * @brief Internal helper to free all allocated path strings.
 *
 * Called by hyLogSaveDisable() or in an error path.
 */
void _hyLogFreePath();

/**
 * @internal
 * @brief Internal helper that handles the file rotation and writing logic.
 *
 * This function is called by _hyLogWrapper() *inside* the mutex.
 * It checks for rotation time, handles error recovery, opens/closes
 * files, and writes the log message to the current file.
 *
 * @param format The log message format string.
 * @param args The va_list of log arguments.
 */
void _hyLogSaveHandle(const char *format, va_list args);

/**
 * @internal
 * @brief Internal helper that manages the log rotation index file.
 *
 * This function is called by _hyLogSaveHandle() *inside* the mutex.
 * It reads the `.log.bin` file (which contains two `int`s: [first_idx, last_idx]),
 * calculates the next file index, deletes the oldest log file if
 * `file_keep_num` is exceeded, and writes the updated indexes back
 * to the `.log.bin` file.
 *
 * @return The integer index for the *new* log file (e.g., 1, 2, 3...),
 * or -1 on any file I/O failure.
 */
int _hyLogNextIndex();

#endif