#include <orangeos.h>
#include <stringdef.h>
#include <wchar.h>

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

size_t
mbstowcs(wchar_t *pwcs, const char *s, size_t n)
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

