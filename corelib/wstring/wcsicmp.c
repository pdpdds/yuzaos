#include <wchar.h>

extern wint_t towlower(wint_t c);

int wcsicmp(const wchar_t* cs, const wchar_t* ct)
{
    while (towlower(*cs) == towlower(*ct))
    {
        if (*cs == 0)
            return 0;
        cs++;
        ct++;
    }
    return towlower(*cs) - towlower(*ct);
}