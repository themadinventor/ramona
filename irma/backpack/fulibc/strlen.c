#include <string.h>

#include "host/irma.h"

size_t strlen(const char *s)
{
#if 1
    return ROM_strlen(s);
#else
	size_t retval;

	for (retval = 0; *s != '\0'; s++)
		retval++;

	return retval;
#endif
}
