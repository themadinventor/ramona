#include <string.h>
#include "host/irma.h"

void *memset(void *dest, int val, size_t count)
{
#if 1
    return ROM_memset(dest, val, count);
#else
	char *temp = (char *)dest;

	for ( ; count != 0; count--)
		*temp++ = val;

	return dest;
#endif
}
