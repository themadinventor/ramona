#include <string.h>

void *memmove(void *dest, const void *src, size_t count)
{
	if ((unsigned int)dest < (unsigned int)src) {
		const char *sp = (const char *)src;
		char *dp = (char *)dest;

		for (; count != 0; count--)
			*dp++ = *sp++;

		return dest;
	} else {
		const char *sp = (const char *)src+count;
		char *dp = (char *)dest+count;

		for (; count != 0; count--)
			*--dp = *--sp;

		return dest;
	}

}
