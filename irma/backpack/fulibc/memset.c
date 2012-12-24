#include <string.h>

void *memset(void *dest, int val, size_t count)
{
	char *temp = (char *)dest;

	for ( ; count != 0; count--)
		*temp++ = val;

	return dest;
}
