#ifndef STDARG_H
#define STDARG_H

#define STACK_WIDTH             sizeof(int)
#define TYPE_WIDTH(TYPE)        ((sizeof(TYPE) + STACK_WIDTH - 1) \
					& ~(STACK_WIDTH - 1))
#define va_rounded_size(TYPE)   \
	(((sizeof(TYPE) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))

#define va_start(PTR, LASTARG)  PTR = (va_list) \
		((char *)&(LASTARG) + TYPE_WIDTH(LASTARG))
#define va_end(PTR)
#define va_arg(PTR, TYPE)       \
	(PTR = (va_list) ((char *) (PTR) + va_rounded_size(TYPE)),\
		*((TYPE *) (void *) ((char *) (PTR) - va_rounded_size(TYPE))))

/* typedef void *va_list; */
#define va_list void *

#endif /* STDARG_H */
