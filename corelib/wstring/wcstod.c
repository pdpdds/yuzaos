#include <wchar.h>
#include <string.h>
#include <corecrt.h>
#include <math.h>
#include <wctype.h>

extern size_t wcsrtombs(char* __restrict dst, const wchar_t** __restrict src, size_t len, mbstate_t* __restrict ps);
extern size_t wcrtomb(char* s, wchar_t wc, mbstate_t* st);

double wcstod(const wchar_t* __restrict nptr, wchar_t** __restrict endptr)
{
	static const mbstate_t initial;
	mbstate_t state;
	double val;
	char* buf, * end, * p;
	const wchar_t* wcp;
	size_t clen, len;

	while (iswspace(*nptr))
		nptr++;

	/*
	 * Convert the supplied numeric wide char. string to multibyte.
	 *
	 * We could attempt to find the end of the numeric portion of the
	 * wide char. string to avoid converting unneeded characters but
	 * choose not to bother; optimising the uncommon case where
	 * the input string contains a lot of text after the number
	 * duplicates a lot of strtod()'s functionality and slows down the
	 * most common cases.
	 */
	state = initial;
	wcp = nptr;
	if ((len = wcsrtombs(NULL, &wcp, 0, &state)) == (size_t)-1) {
		if (endptr != NULL)
			*endptr = (wchar_t*)nptr;
		return (0.0);
	}
	if ((buf = malloc(len + 1)) == NULL)
		return (0.0);
	state = initial;
	wcsrtombs(buf, &wcp, len + 1, &state);

	/* Let strtod() do most of the work for us. */
	val = strtod(buf, &end);

	/*
	 * We only know where the number ended in the _multibyte_
	 * representation of the string. If the caller wants to know
	 * where it ended, count multibyte characters to find the
	 * corresponding position in the wide char string.
	 */
	if (endptr != NULL) {
#if 1					/* Fast, assume 1:1 WC:MBS mapping. */
		* endptr = (wchar_t*)nptr + (end - buf);
		(void)clen;
		(void)p;
#else					/* Slow, conservative approach. */
		state = initial;
		*endptr = (wchar_t*)nptr;
		p = buf;
		while (p < end &&
			(clen = mbrlen(p, end - p, &state)) > 0) {
			p += clen;
			(*endptr)++;
		}
#endif
	}

	free(buf);

	return (val);
}

float wcstof(const wchar_t* __restrict nptr, wchar_t** __restrict endptr)
{
	double retval = wcstod(nptr, endptr);
	if (isnan(retval))
		return nanf(NULL);

	return (float)retval;
}