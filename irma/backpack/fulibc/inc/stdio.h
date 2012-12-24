#ifndef STDIO_H
#define STDIO_H

#include <stddef.h>
#include <stdarg.h>

int printf(const char *, ...);

int sprintf(char *, const char *, ...);

int vprintf(const char *, va_list);
int vsprintf(char *, const char *, va_list);

void putchar(int c);
void puts(const char *);

#endif /* STDIO_H */
