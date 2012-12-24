#include <string.h>

char *strncpy(char *s1, const char *s2, size_t len)
{
	char *dst = s1;
	const char *src = s2;

	while (len && (*dst++ = *src++) != '\0')
		len--;

	while (len--)
		*dst++ = '\0';

	return s1;
}
