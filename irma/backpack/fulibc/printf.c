#include <stdio.h>
#include <stdarg.h>

int printf(const char *format, ...)
{
	va_list args;
	int rv;

	va_start(args, format);
	rv = vprintf(format, args);
	va_end(args);

	return rv;
}
