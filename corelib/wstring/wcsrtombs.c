#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <corecrt.h>


size_t wcsrtombs(char* __restrict dst, const wchar_t** __restrict src, size_t len, mbstate_t* __restrict ps)
{
	char buf[MB_LEN_MAX];
	const wchar_t* s;
	size_t nbytes;
	int nb;

	s = *src;
	nbytes = 0;

	if (dst == NULL) {
		for (;;) {
			if ((nb = (int)wcrtomb(buf, *s, NULL)) < 0)
				/* Invalid character - wcrtomb() sets errno. */
				return ((size_t)-1);
			else if (*s == L'\0')
				return (nbytes + nb - 1);
			s++;
			nbytes += nb;
		}
		/*NOTREACHED*/
	}

	while (len > 0) {
		if (len > (size_t)MB_CUR_MAX) {
			/* Enough space to translate in-place. */
			if ((nb = (int)wcrtomb(dst, *s, NULL)) < 0) {
				*src = s;
				return ((size_t)-1);
			}
		}
		else {
			/* May not be enough space; use temp. buffer. */
			if ((nb = (int)wcrtomb(buf, *s, NULL)) < 0) {
				*src = s;
				return ((size_t)-1);
			}
			if (nb > (int)len)
				/* MB sequence for character won't fit. */
				break;
			memcpy(dst, buf, nb);
		}
		if (*s == L'\0') {
			*src = NULL;
			return (nbytes + nb - 1);
		}
		s++;
		dst += nb;
		len -= nb;
		nbytes += nb;
	}
	*src = s;
	return (nbytes);
}


size_t wcsrtombs2(char* __restrict s, const wchar_t** __restrict ws, size_t n, mbstate_t* __restrict st)
{
	const wchar_t* ws2;
	char buf[4];
	size_t N = n, l;
	if (!s) {
		for (n = 0, ws2 = *ws; *ws2; ws2++) {
			if (*ws2 >= 0x80u) {
				l = wcrtomb(buf, *ws2, 0);
				if (!(l + 1)) return -1;
				n += l;
			}
			else n++;
		}
		return n;
	}
	while (n >= 4) {
		if (**ws - 1u >= 0x7fu) {
			if (!**ws) {
				*s = 0;
				*ws = 0;
				return N - n;
			}
			l = wcrtomb(s, **ws, 0);
			if (!(l + 1)) return -1;
			s += l;
			n -= l;
		}
		else {
			*s++ = **ws;
			n--;
		}
		(*ws)++;
	}
	while (n) {
		if (**ws - 1u >= 0x7fu) {
			if (!**ws) {
				*s = 0;
				*ws = 0;
				return N - n;
			}
			l = wcrtomb(buf, **ws, 0);
			if (!(l + 1)) return -1;
			if (l > n) return N - n;
			wcrtomb(s, **ws, 0);
			s += l;
			n -= l;
		}
		else {
			*s++ = **ws;
			n--;
		}
		(*ws)++;
	}
	return N;
}