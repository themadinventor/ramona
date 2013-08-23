#include <string.h>

#include "host/irma.h"

void *memcpy(void *dest, const void *src, size_t count)
{
#if 1
    return ROM_memcpy(dest, src, count);
#else
	const char *sp = (const char *)src;
	char *dp = (char *)dest;

	for (; count != 0; count--)
		*dp++ = *sp++;

	return dest;
#endif
}
