/**
 * @file core/handle.h
 * @brief Defines the main core application structure and its lifecycle functions.
 *
 * This file ties together all modules (interfaces, peripherals, and apps).
 * It defines the main `Core` struct, which holds all hardware handles,
 * and the `coreLoop` function, which acts as the central "app manager"
 * task for handling application exits and restarts.
 */

#ifndef __CORE_HANDLE_H
#define __CORE_HANDLE_H

#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "haya/log.h"
#include "haya/app.h"
#include "interface.h"
#include "peripheral.h"
#include "service.h"

/**
 * @brief The main application structure.
 *
 * This struct acts as a top-level container for all initialized
 * hardware interfaces and peripherals, making them accessible
 * throughout the application.
 */
typedef struct
{
    CoreInterface interface;   /**< @brief Holds all initialized interface handles (SPI, I2C, etc.). */
    CorePeripheral peripheral; /**< @brief Holds all initialized peripheral handles (NVS, SD, WiFi, etc.). */
    CoreService service;       /**< @brief Holds all initialized service handles (Wifiman, Mqfastt, etc.). */
} Core;

/**
 * @brief Initializes all core systems.
 *
 * This function must be called once at startup from `app_main`.
 * It initializes logging, the `haya_app` manager (including the exit queue),
 * and all interfaces, peripherals, and services based on their
 * respective config files.
 *
 * @param core A pointer to the main Core struct to be populated.
 * @note This function will trigger a device restart if
 * logging or the app manager fails to initialize.
 */
void coreSetup(Core *core);

/**
 * @brief The main application manager loop.
 *
 * This function should be run as a long-lived FreeRTOS task.
 * It waits indefinitely on the global `_hy_app_exit_q` for
 * stopped app handles.
 *
 * When an app handle is received, it logs the exit code and processes
 * the app's `exit_action` (e.g., free, restart, or reboot).
 *
 * @param core A pointer to the initialized Core struct.
 */
void coreLoop(Core *core);

/**
 * @brief Triggers a global device restart.
 *
 * This is a helper function that logs the restart reason and then
 * calls `esp_restart()` after a specified delay.
 *
 * @param wait_ms The delay in milliseconds to wait before restarting.
 */
void coreRestart(uint32_t wait_ms);

/**
 * @internal
 * @brief Frees all resources associated with a stopped app handle.
 *
 * This function is called by `coreLoop` when an app's exit action
 * is `HY_APP_EXIT_ACTION_FREE` or when a restart is not possible.
 * It calls `_hyAppDelete()` to safely free the handle, its name,
 * and its event group.
 *
 * @param app_handle The handle of the *stopped* app to free.
 */
void _coreFreeApp(HyAppHandle *app_handle);

/**
 * @internal
 * @brief Restarts an application that has stopped.
 *
 * This function is called by `coreLoop` when an app's exit action
 * requires a restart. It resets the app's state (exit code, event bits)
 * and calls `hyAppStart()` on the same handle.
 *
 * If `hyAppStart()` fails, it logs the error, frees the app handle,
 * and triggers a device restart.
 *
 * @param app_handle The handle of the *stopped* app to restart.
 */
void _coreRestartApp(HyAppHandle *app_handle);

#endif