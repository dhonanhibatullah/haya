#ifndef __HELLO_WORLD_HELLO_WORLD_H
#define __HELLO_WORLD_HELLO_WORLD_H

#include "haya/log.h"
#include "haya/app.h"

typedef struct
{
    HyAppHandle *app_hdl;
    uint8_t hello_counter;
} HelloWorldApp;

HelloWorldApp *helloWorldAppNew();
int _helloWorldAppSetup(void *arg);
int _helloWorldAppLoop(void *arg);
void _helloWorldAppOnPaused(void *arg);
void _helloWorldAppOnResumed(void *arg);
void _helloWorldAppOnStopped(void *arg);

#endif