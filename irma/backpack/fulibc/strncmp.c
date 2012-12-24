#include <string.h>

int strncmp(const char * const s1, const char * const s2, const size_t num)
{
	const unsigned char * const us1 = (const unsigned char *) s1;
	const unsigned char * const us2 = (const unsigned char *) s2;

	for (size_t i = (size_t)0; i < num; ++i) {
		if (us1[i] < us2[i])
			return -1;
		else if (us1[i] > us2[i])
			return 1;
		else if (!us1[i]) /* null byte -- end of string */
			return 0;
	}

	return 0;
}
