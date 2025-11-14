/**
 * @file haya/app.h
 * @brief An application task wrapper for FreeRTOS.
 *
 * This module provides a simplified abstraction for managing the lifecycle
 * of a FreeRTOS task (e.g., setup, loop, pause, resume, stop).
 * It handles the underlying FreeRTOS synchronization primitives (like tasks
 * and event groups) to ensure a robust, thread-safe state machine.
 *
 * @note The typical usage is to create a handle with hyAppNew(),
 * set callbacks with hyAppSetCallbackGroup(), and manage
 * the task with hyAppStart(), hyAppStop(), etc.
 */

#ifndef __HAYA_APP_H
#define __HAYA_APP_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "haya/error.h"

/**
 * @defgroup haya_app_internal Internal App Defines
 * @brief Internal timeouts and event bits for task synchronization.
 * @internal
 * @{
 */
#define _HY_APP_START_TIMEOUT pdMS_TO_TICKS(1000)         /**< @brief Max time to wait for task to confirm it's running. */
#define _HY_APP_AFTER_SETUP_DELAY pdMS_TO_TICKS(300)      /**< @brief Short delay after on_setup completes before on_loop. */
#define _HY_APP_TOLERANCE_DELAY pdMS_TO_TICKS(1000)       /**< @brief Generic tolerance for state change confirmations. */
#define _HY_APP_EXIT_QUEUE_SEND_DELAY pdMS_TO_TICKS(3000) /**< @brief Safe short delay before sending exit queue. */
#define _HY_APP_EVENT_RUNNING_BIT (1 << 0)                /**< @brief Bit set by task when it's alive (cleared by start). */
#define _HY_APP_EVENT_STOPPED_BIT (1 << 1)                /**< @brief Bit set by task just before it self-deletes. */
#define _HY_APP_EVENT_PAUSED_BIT (1 << 2)                 /**< @brief Bit set by task when it enters the paused state. */
#define _HY_APP_EVENT_RESUMED_BIT (1 << 3)                /**< @brief Bit set by task when it exits the paused state. */
#define _HY_APP_EVENT_RESUME_CMD_BIT (1 << 4)             /**< @brief Bit set by hyAppResume() to command the task to resume. */
#define _HY_APP_EXIT_QUEUE_LEN 2                          /**< @brief Max items in the global app exit queue. */
#define _HY_APP_PASS_CODE 0xFFFFFFFF                      /**< @brief Pass return code for the app. */
/** @} */

/**
 * @brief Abstracted task priorities for FreeRTOS.
 * @note These priorities map directly to FreeRTOS priorities,
 * from tskIDLE_PRIORITY (0) up to (configMAX_PRIORITIES - 1).
 */
typedef enum
{
    HY_APP_PRIORITY_IDLE_0,     /**< Priority 0 (tskIDLE_PRIORITY) */
    HY_APP_PRIORITY_IDLE_1,     /**< Priority 1 */
    HY_APP_PRIORITY_IDLE_2,     /**< Priority 2 */
    HY_APP_PRIORITY_IDLE_3,     /**< Priority 3 */
    HY_APP_PRIORITY_IDLE_4,     /**< Priority 4 */
    HY_APP_PRIORITY_LOW_0,      /**< Priority 5 */
    HY_APP_PRIORITY_LOW_1,      /**< Priority 6 */
    HY_APP_PRIORITY_LOW_2,      /**< Priority 7 */
    HY_APP_PRIORITY_LOW_3,      /**< Priority 8 */
    HY_APP_PRIORITY_LOW_4,      /**< Priority 9 */
    HY_APP_PRIORITY_MODERATE_0, /**< Priority 10 */
    HY_APP_PRIORITY_MODERATE_1, /**< Priority 11 */
    HY_APP_PRIORITY_MODERATE_2, /**< Priority 12 */
    HY_APP_PRIORITY_MODERATE_3, /**< Priority 13 */
    HY_APP_PRIORITY_MODERATE_4, /**< Priority 14 */
    HY_APP_PRIORITY_HIGH_0,     /**< Priority 15 */
    HY_APP_PRIORITY_HIGH_1,     /**< Priority 16 */
    HY_APP_PRIORITY_HIGH_2,     /**< Priority 17 */
    HY_APP_PRIORITY_HIGH_3,     /**< Priority 18 */
    HY_APP_PRIORITY_HIGH_4,     /**< Priority 19 */
    HY_APP_PRIORITY_REALTIME_0, /**< Priority 20 */
    HY_APP_PRIORITY_REALTIME_1, /**< Priority 21 */
    HY_APP_PRIORITY_REALTIME_2, /**< Priority 22 */
    HY_APP_PRIORITY_REALTIME_3, /**< Priority 23 */
    HY_APP_PRIORITY_REALTIME_4  /**< Priority 24 (configMAX_PRIORITIES - 1) */
} HyAppPriority;

/**
 * @brief Defines the action to take after an app task exits.
 *
 * This is set in the HyAppConfig and processed by a manager task
 * that listens to the global `_app_exit_q` queue.
 */
typedef enum
{
    /** @brief Just free the app's resources. Do not restart. */
    HY_APP_EXIT_ACTION_FREE,
    /** @brief Always restart the app, regardless of exit code. */
    HY_APP_EXIT_ACTION_RESTART,
    /** @brief Restart the app *only if* it exited with an error code
     * (i.e., not from a manual hyAppStop() call). */
    HY_APP_EXIT_ACTION_RESTART_UNLESS_STOPPED,
    /** @brief Trigger a system-wide device restart. */
    HY_APP_EXIT_ACTION_RESTART_DEVICE
} HyAppExitAction;

/**
 * @brief A callback function pointer that can return an exit code.
 * @param arg The user-defined parameter, originally passed in HyAppConfig.
 * @return A `HyAppExitCode`. Returning any value other than
 * `HY_APP_EXIT_CODE_NONE` will cause the task to stop.
 */
typedef int (*HyAppCallback)(void *arg);

/**
 * @brief A callback function pointer that does not return a value.
 *
 * Used for callbacks that cannot trigger a task stop (e.g., on_stopped).
 * @param arg The user-defined parameter, originally passed in HyAppConfig.
 */
typedef void (*HyAppVoidCallback)(void *arg);

/**
 * @brief Global queue for notifying of application exits.
 * @internal
 *
 * This queue is defined and created by the app wrapper's C file.
 * When an app task stops, its `HyAppHandle*` is posted to this queue.
 * A central manager task is expected to listen to this queue to
 * process the `HyAppExitAction` for the stopped app.
 */
extern QueueHandle_t _hy_app_exit_q;

/**
 * @brief Configuration structure for creating a new application handle.
 *
 * This structure is filled by the user and passed to hyAppNew().
 */
typedef struct
{
    char *name;                  /**< @brief The name for the FreeRTOS task.
                                  * @note A deep copy of this string is made,
                                  * so the original can be a stack variable. */
    HyAppPriority priority;      /**< @brief Task priority, see HyAppPriority. */
    size_t stack_size;           /**< @brief Stack size for the task, in bytes. */
    TickType_t sleep_tick;       /**< @brief Ticks to delay in the on_loop callback (vTaskDelay). */
    void *param;                 /**< @brief A user-defined parameter that will be passed
                                  * to all callbacks. */
    HyAppExitAction exit_action; /**< @brief Action to take after this task stops.
                                  * Processed by the central app manager. */
} HyAppConfig;

/**
 * @brief A structure to hold all application lifecycle callbacks.
 *
 * @note This struct is initialized by the user and passed to
 * hyAppSetCallbackGroup().
 * @warning The hyAppSetCallbackGroup() function is not thread-safe and
 * should only be called when the task is stopped or paused.
 */
typedef struct
{
    /**
     * @brief Called once after the task starts.
     * @note Returning an exit code (not NONE) will stop the task.
     */
    HyAppCallback on_setup;

    /**
     * @brief The main application loop. Called repeatedly.
     * @note Returning an exit code (not NONE) will stop the task.
     */
    HyAppCallback on_loop;

    /**
     * @brief Called once when the task enters the paused state.
     * @note This callback cannot return an exit code.
     */
    HyAppVoidCallback on_paused;

    /**
     * @brief Called once when the task resumes from pause.
     * @note This callback cannot return an exit code.
     */
    HyAppVoidCallback on_resumed;

    /**
     * @brief Called once when the task is stopping, before it self-deletes.
     * @note This callback cannot return an exit code.
     */
    HyAppVoidCallback on_stopped;
} HyAppCallbackGroup;

/**
 * @brief The main application handle.
 * @note This is an opaque handle. Its members are internal and should
 * not be modified directly by the user.
 */
typedef struct
{
    TaskHandle_t _th;        /**< @internal Task handle for the application. */
    EventGroupHandle_t _ev;  /**< @internal Event group for state synchronization. */
    HyAppConfig _cfg;        /**< @internal A copy of the user's configuration. */
    HyAppCallbackGroup _cb;  /**< @internal A copy of the user's callbacks. */
    volatile int _exit_code; /**< @internal Stores the exit code returned by a callback. */
    volatile bool _ok;       /**< @internal Internal flag to control the main loop (true = run). */
    volatile bool _sus;      /**< @internal Internal flag to control pause state (true = pause). */

} HyAppHandle;

/**
 * @brief Creates a new application handle and allocates resources.
 *
 * This function allocates memory for the handle, its event group, and
 * a deep copy of the task name from the config.
 *
 * @param cfg A pointer to the configuration struct. Must not be NULL.
 * @return A pointer to the new HyAppHandle, or NULL on failure
 * (e.g., out of memory, or if `_hyAppInit` was not called).
 * @note The handle's lifecycle is managed by the app manager
 * (via the `_app_exit_q`) and should not be freed manually.
 */
HyAppHandle *hyAppNew(HyAppConfig *cfg);

/**
 * @brief Sets the callback functions for the application.
 *
 * @param h The application handle.
 * @param cb A pointer to the struct containing the callback function pointers.
 * @return HY_ERR_NONE on success.
 * @return HY_ERR_BAD_ARGS if h or cb are NULL.
 * @warning This function is NOT thread-safe. It should only be called
 * when the application task is stopped or paused.
 */
HyErr hyAppSetCallbackGroup(HyAppHandle *h, HyAppCallbackGroup *cb);

/**
 * @brief Starts the application task.
 *
 * This function creates and starts the FreeRTOS task. It will block
 * for up to _HY_APP_START_TIMEOUT milliseconds waiting for the task
 * to confirm it is running.
 *
 * @param h The application handle.
 * @return HY_ERR_NONE on success.
 * @return HY_ERR_BAD_ARGS if h is NULL.
 * @return HY_ERR_FAILURE if the task could not be created.
 * @return HY_ERR_TIMEOUT if the task failed to start in time.
 */
HyErr hyAppStart(HyAppHandle *h);

/**
 * @brief Signals the application task to stop gracefully.
 *
 * This is a blocking function. It signals the task to stop and waits
 * for confirmation. If the task does not stop gracefully within the
 * timeout period, it will be forcibly deleted.
 *
 * @param h The application handle.
 * @return HY_ERR_NONE on a successful, graceful stop.
 * @return HY_ERR_BAD_ARGS if h is NULL.
 * @return HY_ERR_TIMEOUT if the task did not stop gracefully and
 * required a forced kill.
 * @note When this function returns, the task is guaranteed to be stopped,
 * and its handle will have been posted to the `_app_exit_q` for cleanup.
 */
HyErr hyAppStop(HyAppHandle *h);

/**
 * @brief Signals the application task to pause.
 *
 * This is a blocking function. It signals the task to pause and waits
 * for confirmation.
 *
 * @param h The application handle.
 * @return HY_ERR_NONE on success (or if already paused).
 * @return HY_ERR_BAD_ARGS if h is NULL.
 * @return HY_ERR_TIMEOUT if the task fails to confirm the pause.
 * @note This function is idempotent (it is safe to call on a task
 * that is already paused).
 */
HyErr hyAppPause(HyAppHandle *h);

/**
 * @brief Signals a paused application task to resume.
 *
 * This is a blocking function. It commands the task to resume and waits
 * for confirmation.
 *
 * @param h The application handle.
 * @return HY_ERR_NONE on success (or if already running).
 * @return HY_ERR_BAD_ARGS if h is NULL.
 * @return HY_ERR_TIMEOUT if the task fails to confirm it has resumed.
 * @note This function is idempotent (it is safe to call on a task
 * that is already running).
 */
HyErr hyAppResume(HyAppHandle *h);

/**
 * @brief Gets the exit code from a stopped application handle.
 *
 * This function is used to check *why* an application task stopped.
 * It should be called after `hyAppStop()` returns, or after confirming
 * the task is no longer running (e.g., after receiving it from the
 * `_app_exit_q`).
 *
 * @param h The application handle.
 * @return The integer exit code returned by the callback that triggered
 * the stop, or `_HY_APP_PASS_CODE` if it was stopped
 * manually via `hyAppStop()` or exited normally.
 */
int hyAppExitCode(HyAppHandle *h);

/**
 * @brief Helper function to return a custom exit code from an app callback.
 *
 * Use this in `on_setup` or `on_loop` to stop the task and signal a
 * specific error condition. Any return value other than the one
 * from `hyAppPass()` will cause the task to stop.
 *
 * @code
 * int my_loop(void* arg) {
 * if (sensor_failed()) {
 * return hyAppExit(12); // Return custom error code 12
 * }
 * return hyAppPass();
 * }
 * @endcode
 *
 * @param code The user-defined integer exit code.
 * @return The same exit code.
 */
inline int hyAppExit(int code) { return code; }

/**
 * @brief Helper function to return the "pass" code from an app callback.
 *
 * This function signals that the callback completed successfully and the
 * task should continue running. This is the only return value that
 * will *not* stop the task.
 *
 * @code
 * int my_loop(void* arg) {
 * do_work();
 * return hyAppPass(); // Continue running
 * }
 * @endcode
 *
 * @return The internal pass code (`_HY_APP_PASS_CODE`).
 */
inline int hyAppPass() { return _HY_APP_PASS_CODE; }

/**
 * @internal
 * @brief The main FreeRTOS task wrapper function.
 *
 * This function implements the core state machine (setup, loop,
 * paused, stopped) based on the flags and event bits in the handle.
 *
 * @param pvParameter A void pointer to the HyAppHandle.
 * @note Do not call this function directly. It is passed to xTaskCreate().
 */
void _hyAppTaskWrapper(void *pvParameter);

/**
 * @internal
 * @brief Initializes the app manager module.
 *
 * This function must be called once at startup before any app
 * is created. It initializes the global `_app_exit_q`.
 *
 * @return true on success, false if queue creation fails.
 */
bool _hyAppInit();

/**
 * @internal
 * @brief Destroys a stopped app handle and frees its resources.
 *
 * @warning This function is for internal use by the app manager *only*.
 * It **must only** be called on a handle that has been received
 * from the `_app_exit_q`, as it assumes the task is already dead.
 * It does *not* stop the task; it only frees the handle's memory
 * (the handle itself, the name string, and the event group).
 *
 * @param h The application handle to destroy.
 * @return HY_ERR_NONE on success, or HY_ERR_BAD_ARGS if h is NULL.
 */
HyErr _hyAppDelete(HyAppHandle *h);

#endif