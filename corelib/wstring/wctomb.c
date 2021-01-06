#include <wchar.h>
#include <errno.h>
#include <corecrt.h>

extern size_t wcrtomb(char* s, wchar_t wc, mbstate_t* st);

int wctomb(char* s, wchar_t wc)
{
	if (!s) return 0;
	return wcrtomb(s, wc, 0);
}