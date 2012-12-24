#include <stdio.h>
#include <stdarg.h>

#include "doprintf.h"

static int vsprintf_helper(unsigned c, void **ptr)
{
	char *dst;

	dst = *ptr;
	*dst++ = (char)c;
	*ptr = dst;

	return 0;
}

int vsprintf(char *buf, const char *fmt, va_list args)
{
	int rv;

	rv = do_printf(fmt, args, vsprintf_helper, (void *)buf);
	buf[rv] = '\0';

	return rv;
}
