#include "apps/hello_world/hello_world.h"

const char *HELLO_WORLD_APP_TAG = "hello_world";

HelloWorldApp *helloWorldAppNew()
{
    HelloWorldApp *new = (HelloWorldApp *)malloc(sizeof(HelloWorldApp));
    if (new == NULL)
        return NULL;

    HyAppConfig app_cfg = {
        .name = "hello_world",
        .stack_size = 4096,
        .priority = HY_APP_PRIORITY_MODERATE_0,
        .sleep_tick = pdMS_TO_TICKS(1000),
        .exit_action = HY_APP_EXIT_ACTION_RESTART_UNLESS_STOPPED,
        .param = new,
    };
    HyAppCallbackGroup app_cb = {
        .on_setup = _helloWorldAppSetup,
        .on_loop = _helloWorldAppLoop,
        .on_paused = _helloWorldAppOnPaused,
        .on_resumed = _helloWorldAppOnResumed,
        .on_stopped = _helloWorldAppOnStopped,
    };
    new->app_hdl = hyAppNew(&app_cfg);
    if (new->app_hdl == NULL)
    {
        free(new);
        return NULL;
    }
    hyAppSetCallbackGroup(new->app_hdl, &app_cb);

    new->hello_counter = 0;

    return new;
}

int _helloWorldAppSetup(void *arg)
{
    hyLogInfo(
        HELLO_WORLD_APP_TAG,
        "app started");

    return hyAppPass();
}

int _helloWorldAppLoop(void *arg)
{
    HelloWorldApp *app = (HelloWorldApp *)arg;

    hyLogInfo(
        HELLO_WORLD_APP_TAG,
        "Hello World! count: %u / 5",
        app->hello_counter + 1);

    app->hello_counter += 1;
    if (app->hello_counter == 5)
        return hyAppExit(0);

    return hyAppPass();
}

void _helloWorldAppOnPaused(void *arg)
{
    hyLogInfo(
        HELLO_WORLD_APP_TAG,
        "app paused");
}

void _helloWorldAppOnResumed(void *arg)
{
    hyLogInfo(
        HELLO_WORLD_APP_TAG,
        "app resumed");
}

void _helloWorldAppOnStopped(void *arg)
{
    hyLogInfo(
        HELLO_WORLD_APP_TAG,
        "app stopped");

    HelloWorldApp *app = (HelloWorldApp *)arg;
    app->hello_counter = 0;
}