#include <orangeos.h>
#include <stringdef.h>
#include <wchar.h>
#include <errno.h>

//android ndk musl-multibyte 
//#include "internal.h"

#define OOB(c,b) (((((b)>>3)-0x10)|(((b)>>3)+((int32_t)(c)>>26))) & ~7)

/* Interval [a,b). Either a must be 80 or b must be c0, lower 3 bits clear. */
#define R(a,b) ((uint32_t)((a==0x80 ? 0x40-b : -a) << 23))
#define FAILSTATE R(0x80,0x80)

#define SA 0xc2u
#define SB 0xf4u

#define C(x) ( x<2 ? -1 : ( R(0x80,0xc0) | x ) )
#define D(x) C((x+16))
#define E(x) ( ( x==0 ? R(0xa0,0xc0) : \
                 x==0xd ? R(0x80,0xa0) : \
                 R(0x80,0xc0) ) \
             | ( R(0x80,0xc0) >> 6 ) \
             | x )
#define F(x) ( ( x>=5 ? 0 : \
                 x==0 ? R(0x90,0xc0) : \
                 x==4 ? R(0x80,0xa0) : \
                 R(0x80,0xc0) ) \
             | ( R(0x80,0xc0) >> 6 ) \
             | ( R(0x80,0xc0) >> 12 ) \
             | x )

const uint32_t bittab[] = {
				  C(0x2),C(0x3),C(0x4),C(0x5),C(0x6),C(0x7),
	C(0x8),C(0x9),C(0xa),C(0xb),C(0xc),C(0xd),C(0xe),C(0xf),
	D(0x0),D(0x1),D(0x2),D(0x3),D(0x4),D(0x5),D(0x6),D(0x7),
	D(0x8),D(0x9),D(0xa),D(0xb),D(0xc),D(0xd),D(0xe),D(0xf),
	E(0x0),E(0x1),E(0x2),E(0x3),E(0x4),E(0x5),E(0x6),E(0x7),
	E(0x8),E(0x9),E(0xa),E(0xb),E(0xc),E(0xd),E(0xe),E(0xf),
	F(0x0),F(0x1),F(0x2),F(0x3),F(0x4)
};

#ifndef __WINT_TYPE__
# define __WINT_TYPE__ unsigned int
#endif
/* Conversion state information.  */
typedef struct
{
	int __count;
	union
	{
		__WINT_TYPE__ __wch;
		char __wchb[4];
	} __value;                /* Value so far.  */
} __mbstate_t;

typedef __mbstate_t mbstate_t;


size_t __mbsrtowcs(wchar_t* ws, const char** src, size_t wn, mbstate_t* st)
{
	const unsigned char* s = (const void*)*src;
	size_t wn0 = wn;
	unsigned c = 0;
	if (st && (c = *(unsigned*)st)) {
		if (ws) {
			*(unsigned*)st = 0;
			goto resume;
		}
		else {
			goto resume0;
		}
	}
	if (!ws) for (;;) {
		if (*s - 1u < 0x7f && (uintptr_t)s % 4 == 0) {
			while (!((*(uint32_t*)s | *(uint32_t*)s - 0x01010101) & 0x80808080)) {
				s += 4;
				wn -= 4;
			}
		}
		if (*s - 1u < 0x7f) {
			s++;
			wn--;
			continue;
		}
		if (*s - SA > SB - SA) break;
		c = bittab[*s++ - SA];
	resume0:
		if (OOB(c, *s)) { s--; break; }
		s++;
		if (c & (1U << 25)) {
			if (*s - 0x80u >= 0x40) { s -= 2; break; }
			s++;
			if (c & (1U << 19)) {
				if (*s - 0x80u >= 0x40) { s -= 3; break; }
				s++;
			}
		}
		wn--;
		c = 0;
	}
	else for (;;) {
		if (!wn) return wn0;
		if (*s - 1u < 0x7f && (uintptr_t)s % 4 == 0) {
			while (wn >= 4 && !((*(uint32_t*)s | *(uint32_t*)s - 0x01010101) & 0x80808080)) {
				*ws++ = *s++;
				*ws++ = *s++;
				*ws++ = *s++;
				*ws++ = *s++;
				wn -= 4;
			}
		}
		if (*s - 1u < 0x7f) {
			*ws++ = *s++;
			wn--;
			continue;
		}
		if (*s - SA > SB - SA) break;
		c = bittab[*s++ - SA];
	resume:
		if (OOB(c, *s)) { s--; break; }
		c = (c << 6) | *s++ - 0x80;
		if (c & (1U << 31)) {
			if (*s - 0x80u >= 0x40) { s -= 2; break; }
			c = (c << 6) | *s++ - 0x80;
			if (c & (1U << 31)) {
				if (*s - 0x80u >= 0x40) { s -= 3; break; }
				c = (c << 6) | *s++ - 0x80;
			}
		}
		*ws++ = c;
		wn--;
		c = 0;
	}
	if (!c && !*s) {
		if (ws) {
			*ws = 0;
			*src = 0;
		}
		return wn0 - wn;
	}
	errno = EILSEQ;
	if (ws) *src = (const void*)s;
	return -1;
}

size_t mbstowcs(wchar_t *pwcs, const char *s, size_t n)
{
	mbstate_t state;
	memset(&state, '\0', sizeof state);
	/* Return how many we wrote (or maybe an error).  */
	return __mbsrtowcs(pwcs, &s, n, &state);
}


size_t wcstombs(char *mbstr, const wchar_t *wcstr, size_t count)
{
    int written, bytes;
    uint16_t w;

    for (written = 0; *wcstr && written < (int)count;)
    {
	w = *wcstr;
	
	if (w <= 0x7f)
	    bytes = 1;
	else if (w <= 0x7ff)
	    bytes = 2;
	else if (w <= 0xffff)
	    bytes = 3;
	else
	{
	    return 0;
	}

	if (written + bytes > (int)count)
	    return written;

	if (mbstr != 0)
	    switch (bytes)
	    {
	    case 1:
		mbstr[0] = w & 0x7f;
		break;

	    case 2:
		mbstr[1] = 0x80 | (w & 0x3f);
		w >>= 6;
		mbstr[0] = 0xC0 | (w & 0x1f);
		break;

	    case 3:
		mbstr[2] = 0x80 | (w & 0x3f);
		w >>= 6;
		mbstr[1] = 0x80 | (w & 0x3f);
		w >>= 6;
		mbstr[0] = 0xE0 | (w & 0x0f);
		break;
	    }
	
	written += bytes;
	if (mbstr != 0)
	    mbstr += bytes;
	wcstr++;
    }

    /*if (mbstr != 0)
	*mbstr = 0;*/
    return written;
}

