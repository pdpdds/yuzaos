#include <wchar.h>
#include <corecrt.h>
#include <memory.h>
#include <string.h>
#include <wctype.h>
/*
 * See wcstod() for comments as to the logic used.
 */

extern size_t wcsrtombs(char* __restrict s, const wchar_t** __restrict ws, size_t n, mbstate_t* __restrict st);

long double
wcstold(const wchar_t* __restrict nptr, wchar_t** __restrict endptr)
{
	static const mbstate_t initial;
	mbstate_t mbs;
	long double val;
	char* buf, * end;
	const wchar_t* wcp;
	size_t len;

	while (iswspace(*nptr))
		nptr++;

	wcp = nptr;
	mbs = initial;
	if ((len = wcsrtombs(NULL, &wcp, 0, &mbs)) == (size_t)-1) {
		if (endptr != NULL)
			*endptr = (wchar_t*)nptr;
		return (0.0);
	}
	if ((buf = malloc(len + 1)) == NULL)
		return (0.0);
	mbs = initial;
	wcsrtombs(buf, &wcp, len + 1, &mbs);

	val = strtold(buf, &end);

	if (endptr != NULL)
		*endptr = (wchar_t*)nptr + (end - buf);

	free(buf);

	return (val);
}
