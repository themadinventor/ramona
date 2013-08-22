#include <stdio.h>
#include <stdint.h>

#include "host/app.h"

#include "bsl.h"
#include "spp.h"

void teardown(void)
{
    bsl_teardown();
    spp_teardown();
}

int start(int p)
{
    if (bsl_init() || spp_init()) {
        return -1;
    }

    plugin_teardown(teardown);

    return 0;
}

