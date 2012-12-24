#include <string.h>

char *strcat(char *s1, const char *s2)
{
	char *ptr = s1;

	while (*ptr)
		ptr++;

	while ((*ptr++ = *s2++) != '\0')
		;

	return s1;
}
