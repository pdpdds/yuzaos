#ifndef __STDARG_H
#define	__STDARG_H
#include <va_list.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* width of stack == width of int */
#define	STACKITEM	int

/* round up width of objects pushed on stack. The expression before the
& ensures that we get 0 for objects of size 0. */
#define	VA_SIZE(TYPE)					\
	((sizeof(TYPE) + sizeof(STACKITEM) - 1)	\
		& ~(sizeof(STACKITEM) - 1))

/* &(LASTARG) points to the LEFTMOST argument of the function call
(before the ...) */
//#define	va_start(AP, LASTARG)	\
	//(AP=((va_list)&(LASTARG) + VA_SIZE(LASTARG)))

/* nothing for va_end */
#define va_end(AP)

#define __va_rounded_size(TYPE) \
	(((sizeof (TYPE) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))

#define va_start(AP, LASTARG) \
	(AP = ((va_list) &(LASTARG) + __va_rounded_size (LASTARG)))

#define va_arg(AP, TYPE) \
	(AP += __va_rounded_size (TYPE), *((TYPE *) (AP - __va_rounded_size (TYPE))))

//#define va_arg(AP, TYPE)	\
	//(AP += VA_SIZE(TYPE), *((TYPE *)(AP - VA_SIZE(TYPE))))

#ifdef __cplusplus
}
#endif

#endif

