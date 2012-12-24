#include <stdio.h>
#include "uart.h"

void putchar(int c)
{
    if (c == '\n') {
        UART2PutChar('\r');
    }

    UART2PutChar(c);
}

void puts(const char *s)
{
    for (; *s; s++) {
        putchar(*s);
    }
}
