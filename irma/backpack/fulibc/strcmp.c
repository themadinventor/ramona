#include <string.h>
#include "host/irma.h"

int strcmp(const char *s1, const char *s2)
{
#if 1
    return ROM_strcmp(s1, s2);
#else
	unsigned char uc1, uc2;
	while (*s1 != '\0' && *s1 == *s2) {
		s1++;
		s2++;
	}

	uc1 = (*(unsigned char *) s1);
	uc2 = (*(unsigned char *) s2);

	return (uc1 < uc2) ? -1 : (uc1 > uc2);
#endif
}
