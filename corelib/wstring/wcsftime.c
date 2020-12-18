#include <wchar.h>
#include <corecrt.h>
#include <errno.h>
#include <string.h>
#include <size_t.h>

extern size_t mbstowcs(wchar_t* pwcs, const char* s, size_t n);
extern size_t __mbsrtowcs(wchar_t* ws, const char** src, size_t wn, mbstate_t* st);
extern size_t wcsrtombs(char* __restrict s, const wchar_t** __restrict ws, size_t n, mbstate_t* __restrict st);

size_t
wcsftime(wchar_t* __restrict wcs, size_t maxsize,
	const wchar_t* __restrict format, const struct tm* __restrict timeptr)
{
	static const mbstate_t initial;
	mbstate_t mbs;
	char* dst, * dstp, * sformat;
	size_t n, sflen;
	int sverrno;

	sformat = dst = NULL;

	/*
	 * Convert the supplied format string to a multibyte representation
	 * for strftime(), which only handles single-byte characters.
	 */
	mbs = initial;
	sflen = wcsrtombs(NULL, &format, 0, &mbs);
	if (sflen == (size_t)-1)
		goto error;
	if ((sformat = malloc(sflen + 1)) == NULL)
		goto error;
	mbs = initial;
	wcsrtombs(sformat, &format, sflen + 1, &mbs);

	/*
	 * Allocate memory for longest multibyte sequence that will fit
	 * into the caller's buffer and call strftime() to fill it.
	 * Then, copy and convert the result back into wide characters in
	 * the caller's buffer.
	 */
	//20201212
	if (SSIZE_MAX / MB_CUR_MAX <= maxsize) {
		/* maxsize is prepostorously large - avoid int. overflow. */
		errno = EINVAL;
		goto error;
	}
	if ((dst = malloc(maxsize * MB_CUR_MAX)) == NULL)
		goto error;
	if (strftime(dst, maxsize, sformat, timeptr) == 0)
		goto error;
	dstp = dst;
	mbs = initial;
	n = __mbsrtowcs(wcs, (const char**)&dstp, maxsize, &mbs);
	if (n == (size_t)-2 || n == (size_t)-1 || dstp != NULL)
		goto error;

	free(sformat);
	free(dst);
	return (n);

error:
	sverrno = errno;
	free(sformat);
	free(dst);
	errno = sverrno;
	return (0);
}