#include "composition/main/launcher.h"

#include <string.h>

#include "composition/main/application.h"
#include "composition/main/driver.h"
#include "composition/main/infrastructure.h"
#include "composition/main/presentation.h"
#include "composition/main/types.h"
#include "domain/models/error.h"
#include "esp_log.h"

#define TAG_PATH "main/launcher"

static cmp_main_launcher_t main_launcher;

void cmp_main_launcher(void) {
    const char* tag = TAG_PATH;

    bool init_driver         = false;
    bool init_infrastructure = false;
    bool init_application    = false;
    bool init_presentation   = false;

    memset(&main_launcher, 0, sizeof(cmp_main_launcher_t));

    dom_models_error_t err = cmp_main_driver_init(&main_launcher);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ESP_LOGE(tag, "Failed to initialize driver composition: %s", dom_models_error_str(err));
        goto fail;
    }
    init_driver = true;

    err = cmp_main_infrastructure_init(&main_launcher);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ESP_LOGE(tag, "Failed to initialize infrastructure composition: %s", dom_models_error_str(err));
        goto fail;
    }
    init_infrastructure = true;

    err = cmp_main_application_init(&main_launcher);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ESP_LOGE(tag, "Failed to initialize application composition: %s", dom_models_error_str(err));
        goto fail;
    }
    init_application = true;

    err = cmp_main_presentation_init(&main_launcher);
    if (err != DOMAIN_MODELS_ERROR_OK) {
        ESP_LOGE(tag, "Failed to initialize presentation composition: %s", dom_models_error_str(err));
        goto fail;
    }
    init_presentation = true;

    ESP_LOGI(tag, "Main composition initialized");

    return;

fail:
    if (init_presentation) {
        cmp_main_presentation_deinit(&main_launcher);
    }
    if (init_application) {
        cmp_main_application_deinit(&main_launcher);
    }
    if (init_infrastructure) {
        cmp_main_infrastructure_deinit(&main_launcher);
    }
    if (init_driver) {
        cmp_main_driver_deinit(&main_launcher);
    }
}
