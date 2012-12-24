#ifndef __DOPRINTF_H
#define __DOPRINTF_H

typedef int (*fnptr_t) (unsigned c, void **helper);

int do_printf(const char *fmt, va_list args, fnptr_t fn, void *ptr);

#endif
