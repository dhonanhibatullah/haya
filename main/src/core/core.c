#include "core/core.h"

void coreRun()
{
    Core core;
    coreSetup(&core);

    // ____[ USER APPS START ]____

    coreLoop(&core);
}