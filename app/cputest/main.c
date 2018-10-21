#include <stdio.h>
#include <stdint.h>

#include "host/ose.h"

void bench()
{
    unsigned int start = OSE_get_ticks();

    for (int i = 0; i < 0xffffff; i++) ;

    unsigned int stop = OSE_get_ticks();

    printf("cputest: %d ticks\n", stop - start);
}

void set_cpu_clock(int mode)
{
    if (mode == 1) {
        printf("cputest: switching to 2x speed\n");

        unsigned short m = *((unsigned short *) 0x00800938);
        m &= ~8;
        m |= 4;
        *((unsigned short *) 0x00800938) = m;

        m = *((unsigned short *) 0x00800938);
        m |= 2;
        *((unsigned short *) 0x00800938) = m;

        printf("cputest: now running at 2x speed\n");
    }
}

int start(int p)
{
    printf("cputest: start\n");

    //printf("cputest: Running test in default speed:\n");
    //bench();
    
    set_cpu_clock(1);
    printf("cputest: Running test in 2x speed:\n");
    bench();

    return -1;
}

