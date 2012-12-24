#include <stdio.h>
#include <stdarg.h>

int sprintf(char *s, const char *format, ...)
{
	va_list args;
	int rv;

	va_start(args, format);
	rv = vsprintf(s, format, args);
	va_end(args);

	return rv;
}
