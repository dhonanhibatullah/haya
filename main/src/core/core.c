#include "core/core.h"
#include "apps/hello_world/hello_world.h"

const char *CORE_TAG = "core";

void coreRun()
{
    Core core;
    coreSetup(&core);

    HelloWorldApp *hello_world = helloWorldAppNew();
    if (hello_world == NULL)
    {
        hyLogError(
            CORE_TAG,
            "failed to create hello_world app");
        coreRestart(3000);
    }
    HyErr herr = hyAppStart(hello_world->app_hdl);
    if (herr != HY_ERR_NONE)
    {
        hyLogError(
            CORE_TAG,
            "failed to start hello_world app: %s",
            hyErrToStr(herr));
        coreRestart(3000);
    }

    coreLoop(&core);
}