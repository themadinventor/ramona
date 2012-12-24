#include <string.h>

size_t strlen(const char *s)
{
	size_t retval;

	for (retval = 0; *s != '\0'; s++)
		retval++;

	return retval;
}
