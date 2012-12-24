#ifndef STRING_H
#define STRING_H

#include <stddef.h>

int      memcmp(const void *, const void *, size_t);
void    *memcpy(void *, const void *, size_t);
void    *memset(void *, int, size_t);
void    *memmove(void *, const void *, size_t);

char    *strcat(char *, const char *);
size_t   strlen(const char *);
char    *strncpy(char *, const char *, size_t);
int      strcmp(const char *, const char *);
char    *strchr(const char *, int);
int      strncmp(const char * const, const char * const, const size_t);
char    *strstr(const char *, const char *);

#endif /* STRING_H */
