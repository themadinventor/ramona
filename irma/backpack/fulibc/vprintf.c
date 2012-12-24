#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "doprintf.h"

#include "uart.h"

int vprintf_help(unsigned c, void **ptr)
{
	(void) ptr;
	putchar(c);
	return 0;
}

int vprintf(const char *format, va_list args)
{
	return do_printf(format, args, vprintf_help, NULL);
}
