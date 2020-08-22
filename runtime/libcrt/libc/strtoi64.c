#include <minwindef.h>
#include <stddef.h>
#include <wchar.h>
#include <ctype.h>


__int64 strtoi64(const char * 	nptr,
	char ** 	endptr,
	int 	base
)
{
	const char *p = nptr;
	BOOL negative = FALSE;
	BOOL got_digit = FALSE;
	__int64 ret = 0;


/*
	if (!MSVCRT_CHECK_PMT(base == 0 || base >= 2)) return 0;
	if (!MSVCRT_CHECK_PMT(base <= 36)) return 0;*/

	while (isspace(*nptr)) nptr++;

	if (*nptr == '-') {
		negative = TRUE;
		nptr++;
	}
	else if (*nptr == '+')
		nptr++;

	if ((base == 0 || base == 16) && *nptr == '0' && tolower(*(nptr + 1)) == 'x') {
		base = 16;
		nptr += 2;
	}

	if (base == 0) {
		if (*nptr == '0')
			base = 8;
		else
			base = 10;
	}

	while (*nptr) {
		char cur = tolower(*nptr);
		int v;

		if (isdigit(cur)) {
			if (cur >= '0' + base)
				break;
			v = cur - '0';
		}
		else {
			if (cur < 'a' || cur >= 'a' + base - 10)
				break;
			v = cur - 'a' + 10;
		}
		got_digit = TRUE;

		if (negative)
			v = -v;

		nptr++;

		if (!negative && (ret > _I64_MAX / base || ret*base > _I64_MAX - v)) {
			ret = _I64_MAX;

		}
		else if (negative && (ret < _I64_MIN / base || ret*base < _I64_MIN - v)) {
			ret = _I64_MIN;

		}
		else
			ret = ret*base + v;
	}

	if (endptr)
		*endptr = (char*)(got_digit ? nptr : p);

	return ret;
}