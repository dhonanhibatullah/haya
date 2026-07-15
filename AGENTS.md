# Project Goals

The project goal is currently empty

# Project Structure Guide

This file describes the active project structure and the patterns that must be preserved when adding or changing features. The `Project Goals` section above is intentionally the first section so teams can reuse this repository as a template and replace only that section with project-specific goals.

## Architecture Overview

This project follows a layered C architecture for ESP-IDF firmware:

1. `domain`: pure business types and contracts.
2. `infrastructure`: concrete implementations of domain contracts.
3. `application`: usecase implementations that orchestrate domain contracts.
4. `presentation`: external interfaces and task presenters that call application usecases.
5. `composition`: startup wiring, driver initialization, configuration, and lifecycle ordering.

Dependency direction must remain one-way:

```text
presentation -> application -> domain
infrastructure -> domain
composition -> driver/infrastructure/application/presentation/domain
```

The domain layer must not include ESP-IDF headers, infrastructure headers, application headers, presentation headers, or composition headers. Application code may depend on domain contracts and models, but must not call ESP-IDF directly. Presentation code may depend on application/domain interfaces and transport-specific APIs such as HTTP server or FreeRTOS. Composition is the only layer that wires concrete implementations together.

## Layer Guide

### Domain Layer

Location:

- `main/include/domain/models`
- `main/include/domain/contracts`
- `main/include/domain/usecases`

Responsibilities:

- Define reusable data models, error enums, and constants.
- Define contracts as `ctx` plus function-pointer tables.
- Define usecase interfaces without deciding how they are implemented.
- Keep the domain independent from ESP-IDF, storage, network stacks, FreeRTOS tasks, and HTTP.

Patterns:

- Model prefix: `dom_models_*`.
- Contract prefix: `dom_contracts_*`.
- Usecase prefix: `dom_usecases_*`.
- Use `dom_models_error_t` for reusable errors. Do not add feature-specific error strings or context-specific error payloads.
- Add new reusable errors only when they can apply across multiple contexts.
- Contracts and usecases allocate only their interface object with static inline `*_new(void* ctx)` helpers and release it with static inline `*_delete(...)`.
- Contract implementations own the concrete context allocation and assign function pointers after creating the contract object.
- Keep comments short and only where they clarify behavior or async semantics.

Domain models:

- Place cross-layer data structures in `domain/models`.
- Use fixed-size buffers for values that cross API boundaries when the existing model does so.
- Prefer explicit `*_available`, `*_set`, `count`, and `truncated` fields over implicit sentinel values.
- Keep validation policy close to the application or implementation layer unless the model itself defines universal limits.

Domain contracts:

- Place hardware, storage, network, logging, or external-service abstractions in `domain/contracts/<area>`.
- Contracts should describe what the caller needs, not how a backend works.
- A contract method returns `dom_models_error_t` unless it is intentionally fire-and-forget, such as logger methods.
- Use `get_` for read operations, `set_` for writes, and `clear_` or `forget_` when existing naming already establishes that behavior.

Domain usecases:

- Place application-facing interfaces in `domain/usecases`.
- Do not place implementation config structs in the domain usecase header.
- Keep usecase methods presentation-neutral. HTTP status codes, JSON shapes, and task timing do not belong in domain usecases.

### Infrastructure Layer

Location:

- `main/include/infrastructure`
- `main/src/infrastructure`

Responsibilities:

- Implement domain contracts using ESP-IDF, NVS, stdio, stubs, or other concrete backends.
- Translate backend-specific errors into `dom_models_error_t`.
- Own concrete context allocation, backend handles, runtime flags, and backend-specific helper functions.
- Provide stub implementations for features that need tests, local simulation, or non-hardware fallback.

File pattern:

```text
main/include/infrastructure/<area>/<feature>/<backend>_impl.h
main/include/infrastructure/<area>/<feature>/<backend>_impl_types.h
main/include/infrastructure/<area>/<feature>/<backend>_impl_utils.h
main/src/infrastructure/<area>/<feature>/<backend>_impl.c
main/src/infrastructure/<area>/<feature>/<backend>_impl_utils.c
```

Use this split consistently:

- `*_impl.h`: public constructor/destructor and optional lifecycle functions.
- `*_impl_types.h`: config structs, default macros, context structs, backend constants.
- `*_impl_utils.h/.c`: helper functions, validation, copying, error conversion, backend mapping.
- `*_impl.c`: constructor/destructor, function-pointer assignment, and contract method implementations.

Constructor pattern:

- Constructor name: `inf_<area>_<feature>_<backend>_impl_new`.
- Destructor name: `inf_<area>_<feature>_<backend>_impl_delete`.
- Allocate the backend context first.
- If `cfg == NULL`, copy a local default config macro when one exists.
- Create the domain contract object with `dom_contracts_*_new(ctx)`.
- Assign every required contract function pointer before returning.
- On any allocation or config failure, release everything already allocated.

Lifecycle pattern:

- Add explicit `*_init` and `*_deinit` only when the backend has a meaningful external lifecycle, event registration, driver binding, or idempotent start/stop preparation.
- Keep `initialized`, `started`, and registration flags in the backend context.
- `*_delete` should tolerate `NULL` and should call deinit when the backend owns registered resources.

Stub pattern:

- Stub implementations use the same domain contract as real implementations.
- Stub config should be deterministic and should not require ESP-IDF hardware state.
- Stub repositories should keep runtime copies of strings and credentials, not borrowed pointers.

Error handling:

- Convert `esp_err_t` in helper functions such as `*_error_from_esp`.
- Return `DOMAIN_MODELS_ERROR_BAD_ARGUMENT` for invalid pointers or invalid caller inputs.
- Return `DOMAIN_MODELS_ERROR_BAD_STATE` when the object exists but the requested operation is not valid for its state.
- Return `DOMAIN_MODELS_ERROR_NOT_FOUND` for absent records or unavailable interfaces.

### Application Layer

Location:

- `main/include/application`
- `main/src/application`

Responsibilities:

- Implement domain usecases.
- Orchestrate domain contracts into business/application flows.
- Own usecase runtime state and policy such as retry counts, AP auto-management, commit state, and event callback registration.
- Log meaningful success and failure events through the logger contract.

File pattern:

```text
main/include/application/<usecase>/impl.h
main/include/application/<usecase>/impl_types.h
main/include/application/<usecase>/impl_utils.h
main/src/application/<usecase>/impl.c
main/src/application/<usecase>/impl_utils.c
```

Application rules:

- Config structs belong in `application/<usecase>/impl_types.h`, not in `domain/usecases`.
- Application code depends on domain contracts and models, not infrastructure implementations.
- Application code should not include ESP-IDF headers unless there is no reasonable alternative; prefer keeping ESP-IDF behind infrastructure or presentation.
- Validate required contract function pointers in `impl_utils.c`.
- Constructor returns the domain usecase interface and assigns all usecase function pointers.
- Destructor unregisters callbacks, frees application context, and deletes the usecase interface.

Logging style:

- Define `#define BASE_TAG "<usecase>"`.
- Inside each function, use `const char* tag = BASE_TAG"/function_name";`.
- Use capitalized first-letter log messages.
- Put hardcoded message text directly in the log format string.
- Log every error case with the domain error string and numeric error when an error value exists:

```c
ctx->cfg.logger->error(ctx->cfg.logger, tag, "Failed to start WiFi: %s (%d)", dom_models_error_str(err), (int)err);
```

- Log successful high-level operations, but avoid noisy logs inside tight loops unless needed for diagnosis.

### Presentation Layer

Location:

- `main/include/presentation/http`
- `main/src/presentation/http`
- `main/include/presentation/task`
- `main/src/presentation/task`

Responsibilities:

- Expose application usecases through transport-specific interfaces.
- Keep parsing, response generation, route registration, and task scheduling out of the application layer.
- Convert presentation-level errors to transport responses while preserving domain error meaning.

HTTP DTO pattern:

- Common JSON helpers live in `presentation/http/dto/common.*`.
- Feature-specific JSON parse/build helpers live in `presentation/http/dto/<feature>.*`.
- DTO functions should convert between JSON and domain/application structs.
- Delete `cJSON` objects after use.
- Keep JSON shape decisions in DTO files, not handlers or usecases.

HTTP handler pattern:

- Handler types live in `presentation/http/handler/<feature>_types.h`.
- Handler functions receive `httpd_req_t*`.
- The route layer passes a pointer to the handler struct as `req->user_ctx`.
- Handlers validate `req`, `user_ctx`, and required usecase pointers through local helpers.
- Handlers call usecase methods and return JSON or domain-error responses through DTO common helpers.
- For operations that can disrupt network connectivity, send the HTTP response first, then trigger the disruptive operation.

HTTP route pattern:

- Routes live in `presentation/http/route/<feature>.*`.
- Keep a static route table mapping URI, method, and handler.
- Expose exactly:
  - `pres_http_route_<feature>_register(server, handler)`
  - `pres_http_route_<feature>_unregister(server)`
  - `pres_http_route_<feature>_route_cnt(void)`
- Register routes with the handler struct as `user_ctx`.
- On partial registration failure, composition should unregister the route set before failing.

Task presentation pattern:

- FreeRTOS task presenters live under `presentation/task/<feature>`.
- Use `types.h`, `utils.h/.c`, and the main task implementation file.
- Expose `new`, `delete`, `start`, and `stop`.
- The task owns scheduling, delays, task handles, and stop flags.
- The task should call a small application/usecase API rather than duplicating application logic.

### Composition Layer

Location:

- `main/include/composition/main`
- `main/src/composition/main`

Responsibilities:

- Own compile-time feature toggles and constant runtime config.
- Initialize ESP-IDF global drivers and low-level peripherals.
- Create infrastructure implementations and bind them to domain contracts.
- Create application usecase implementations from infrastructure contracts.
- Register presentation routes and start presentation tasks.
- Preserve deterministic init/deinit order.

Composition files:

- `config.h`: compile-time `COMPOSITION_MAIN_CONFIG_*` toggles and config struct definitions.
- `config.c`: constant config values and compile-time selected defaults.
- `types.h`: launcher state grouped by `driver`, `infrastructure`, `application`, and `presentation`.
- `driver.c`: ESP-IDF global systems and low-level resources.
- `infrastructure.c`: concrete contract implementations.
- `application.c`: application usecase construction/startup.
- `presentation.c`: route registration and task startup.
- `launcher.c`: top-level init sequence and partial-failure cleanup.
- `preloaded.c`: startup data loaded before higher-level composition.

Init order:

```text
driver -> infrastructure -> application -> presentation
```

Deinit order:

```text
presentation -> application -> infrastructure -> driver
```

Composition rules:

- Guard feature fields and code with `#ifdef COMPOSITION_MAIN_CONFIG_*`.
- Keep init flags per resource so partial initialization can be safely deinitialized.
- On init failure, call that layer's deinit before returning the error when resources may already exist.
- Use `ESP_LOG*` in composition and driver code.
- Do not put application policy in composition; composition only wires config and dependencies.
- Do not allocate mutable config dynamically. Config values live directly in `cmp_main_config`.
- Lists in config, such as GPIO definitions, should be `static const` arrays in `config.c`, with counts calculated by `sizeof(array) / sizeof(array[0])`.

## Code Style and Naming

General style:

- C code uses include guards and `extern "C"` blocks in headers.
- Keep files ASCII unless a file already uses another encoding for a clear reason.
- Prefer explicit prefixes over generic names.
- Use local `/* Section */` comments to group large files.
- Keep comments short and useful; avoid narrating obvious assignments.
- Prefer deterministic cleanup over process-level assumptions.

Naming prefixes:

- Domain models: `dom_models_*`
- Domain contracts: `dom_contracts_*`
- Domain usecases: `dom_usecases_*`
- Infrastructure: `inf_*`
- Application: `app_*`
- HTTP presentation: `pres_http_*`
- Task presentation: `pres_task_*`
- Main composition: `cmp_main_*`

Function and type conventions:

- Public type names end in `_t`.
- Config structs end in `_cfg_t`.
- Runtime context structs end in `_ctx_t`.
- Default config macros use `*_CFG_DEFAULT()`.
- Constructors use `*_new`.
- Destructors use `*_delete`.
- Lifecycle functions use `*_init`, `*_deinit`, `start`, and `stop` according to the layer's existing convention.

Memory ownership:

- Constructors own allocations they create.
- Destructors must tolerate `NULL`.
- Runtime string values should be copied when the implementation must retain them.
- Caller-provided output buffers must be checked for `NULL` and size where applicable.

Error policy:

- Use `DOMAIN_MODELS_ERROR_OK` for success.
- Use the most specific reusable domain error available.
- Do not introduce feature-specific domain errors.
- Infrastructure converts backend errors; application logs and propagates domain errors; presentation converts domain errors to transport responses.

## How to Add a New Feature

Use this order unless the feature is strictly internal to a lower layer:

1. Add or update domain models for any data crossing layer boundaries.
2. Add or update a domain contract when the application needs a new hardware/storage/network/external capability.
3. Add or update a domain usecase when presentation needs a new application workflow.
4. Implement real infrastructure under `infrastructure/<area>/<feature>/<backend>_impl.*`.
5. Implement a stub backend when the feature has stateful behavior, external dependencies, or future test value.
6. Implement application logic under `application/<usecase>` when behavior combines multiple contracts or owns policy.
7. Implement presentation DTO, handler, route, or task code only after the usecase interface is stable.
8. Wire config, types, init, and deinit in composition.
9. Update this file when the new feature creates a new pattern or changes a layer boundary.

Feature addition checklist:

- Domain remains independent from ESP-IDF.
- Application does not include infrastructure headers.
- Presentation does not duplicate application policy.
- Infrastructure implements domain contracts and translates backend errors.
- Composition owns concrete implementation choice and lifecycle.
- Real and stub implementations use the same contract where possible.
- New config values are constant fields in `cmp_main_config`.
- New optional features have `COMPOSITION_MAIN_CONFIG_*` guards.
- New resources have reverse-order cleanup.

## Existing Features

### Driver Composition

Driver composition initializes low-level ESP-IDF resources and stores raw handles in `cmp_main_driver_t`.

Currently covered:

- ISR service.
- GPIO configuration list with mode, pull-up/down, interrupt type, and initial output level.
- I2C 0 and I2C 1 master buses.
- SPI 2 and SPI 3 host buses.
- NVS flash and project namespace handle.
- Preloaded startup data.
- LittleFS mount.
- SPI-based SD card mount through SDSPI.
- ESP event loop.
- ESP netif.
- ESP WiFi driver.
- W5500 Ethernet driver pieces.
- BLE NimBLE initialization.
- HTTP server startup.
- MQTT client state placeholder.

Driver composition should stay limited to global driver/runtime setup. Higher-level protocols and business behavior belong above it.

### Preloaded Data

Preloaded data is startup data needed before the application layer runs.

- `device_id` and `device_id_str` are derived from the base MAC.
- Default values are loaded first.
- NVS values override defaults when NVS is enabled.
- The global preloaded model is used to seed repository behavior and application defaults.
- `device_id` is read-only from repository perspective because it is determined by hardware identity.

### Logger

The leveled stdio logger implements `dom_contracts_logger_leveled_t`.

- Supports error, warn, info, and debug logging.
- Optional callbacks can receive formatted log messages.
- Application logging depends on this contract, not on ESP logging.

### WiFi Device

The WiFi device contract abstracts WiFi operations:

- Start/stop WiFi.
- Set mode.
- Get status.
- Connect/disconnect STA.
- Start/stop AP.
- Start scan and retrieve scanned results.
- Add/remove event callbacks.

The ESP WiFi implementation wraps ESP-IDF WiFi and event registration. The stub implementation preserves the same contract for non-hardware fallback.

### Network Interface

The network interface contract reads network state:

- `get_all` for all interfaces.
- `get_wifi_sta` for the STA interface.

The ESP netif implementation reads by interface key. The stub implementation provides deterministic fallback state.

### Repositories

Preloaded repository:

- Reads all preloaded values.
- Sets mutable preloaded values.
- Does not set `device_id`.
- Has NVS and stub implementations.

WiFi repository:

- Stores the latest STA credential.
- Supports get, set, and clear.
- Has NVS and stub implementations.

Repositories should not perform application workflows. They only persist and retrieve data.

### WiFiMan Application

WiFiMan is the current primary usecase. It combines:

- Leveled logger.
- WiFi device.
- WiFi credential repository.
- Preloaded repository.
- Network interface reader.

WiFiMan supports:

- Scanning nearby access points.
- Getting scan results.
- STA connect and disconnect.
- Stored STA credential get, set, connect, and forget.
- Status reporting, including WiFi status, STA interface state, stored credential state, reconnect counters, AP auto-management state, and commit state.
- Reconnect decision and reconnect attempt methods for task presentation.
- AP+STA convenience behavior.
- AP auto-management when configured.
- Event callback registration for WiFi state changes without coupling the application to ESP-IDF.

AP auto-management behavior:

- AP is enabled when no stored credential exists or STA connection fails.
- AP is re-enabled after repeated reconnection failure.
- AP is disabled automatically after reconnection succeeds.
- AP is not automatically disabled after an initial user connection until the user commits the STA connection.

### HTTP Presentation

WiFiMan HTTP presentation exposes the application through `/api/wifi/*` routes.

Current route set:

- `GET /api/wifi/status`
- `POST /api/wifi/scan`
- `GET /api/wifi/scan`
- `POST /api/wifi/sta`
- `POST /api/wifi/sta/stored`
- `DELETE /api/wifi/sta`
- `POST /api/wifi/sta/commit`
- `GET /api/wifi/sta/credential`
- `POST /api/wifi/sta/credential`
- `DELETE /api/wifi/sta/credential`
- `GET /api/wifi/reconnect/need`
- `POST /api/wifi/reconnect`

Handlers use DTO helpers for JSON and common domain-error responses. Network-disruptive operations respond first, then call the usecase method.

### Task Presentation

The WiFiMan STA reconnect task owns periodic reconnect scheduling.

- It receives the WiFiMan usecase in its config.
- It calls `need_reconnect`.
- If needed, it calls `try_reconnect`.
- It owns FreeRTOS task creation, stop signaling, interval delay, and task deletion.

### System Update & Restart Contracts

The system update and restart contracts abstract the underlying partition flashing and board reboot logic.

- **System Update**: The `dom_contracts_system_update_t` contract specifies partition flashing, image validation, and partition rollbacks. The ESP HTTPS implementation leverages the native `esp_http_client` and `esp_ota_ops` APIs to download and flash updates.
- **System Restart**: The `dom_contracts_system_restart_t` contract specifies the system reboot mechanism. Its ESP implementation accepts a configurable delay in milliseconds, allowing the logger task or socket buffers to flush before calling `esp_restart()`.

### OTA Application

The OTA application orchestrates firmware update, partition validation, and fallback rollback flows.

- It implements the `dom_usecases_ota_t` usecase.
- It validates incoming metadata (URL, size, SHA256 checksum).
- Upon successful execution of a partition update, it automatically requests a forced reboot (with a 5-second delay) through the system restart contract.

### MQTT Presentation

The MQTT presentation layer handles remote device management commands.

- It routes subscription topics `/sub/{device_id_str}/reset` and `/sub/{device_id_str}/ota` to their respective settings and OTA usecases.
- To prevent starving the MQTT client event loop, incoming OTA updates are spawned and run asynchronously in a separate FreeRTOS background task.
- The MQTT client input/output buffer sizes are configurable via the composition layer (set to 4096 bytes by default).

## Build and Verification Notes

Use ESP-IDF 6 for this project. The W5500 managed component currently requires `idf >=6.0`, so IDF 5.x is not a valid verification target for the active dependency set.

Recommended checks after source changes:

```sh
git diff --check
idf.py build
```

If `idf.py` is not available, source the intended ESP-IDF export script first. If the IDF 6 Python environment is missing, set up the ESP-IDF tools before treating build failure as a project code failure.
