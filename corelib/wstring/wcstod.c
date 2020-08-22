#include <stdio.h>
#include <minwindef.h>
#include <corecrt.h>   
#include <math.h>

#define SLD_UNDERFLOW 1
#define SLD_OVERFLOW 2
#define SLD_NODIGITS 4

typedef enum {
    INTRNCVT_OK,
    INTRNCVT_OVERFLOW,
    INTRNCVT_UNDERFLOW
} INTRNCVT_STATUS;

/*double __cdecl wcstod(const wchar_t* nptr, wchar_t** endptr) {
    double tmp;
    const wchar_t* ptr = nptr;
    char* cptr;
    int len;
    _LDBL12 ld12;
    const char* EndPtr;
    DWORD flags;
    INTRNCVT_STATUS intrncvt;
   
    while (isspace((char)*ptr))
        ptr++;
    if (!(cptr = (char*)LocalAlloc(LPTR, (wcslen(ptr) + 1) * sizeof(wchar_t))))
        return 0.0;
    for (len = 0; ptr[len]; len++)
        cptr[len] = (char)ptr[len];
    cptr[len] = 0;
    flags = __strgtold12(&ld12, &EndPtr, cptr);
    LocalFree(cptr);
    if (flags & SLD_NODIGITS) {
        if (endptr)
            *endptr = (wchar_t*)nptr;
        return 0.0;
    }
    if (endptr)
        *endptr = (wchar_t*)ptr + (EndPtr - cptr);
    intrncvt = _ld12tod(&ld12, &tmp);
    if (flags & SLD_OVERFLOW || intrncvt == INTRNCVT_OVERFLOW)
        return (*ptr == '-') ? -HUGE_VAL : HUGE_VAL; // negative or positive overflow
    if (flags & SLD_UNDERFLOW || intrncvt == INTRNCVT_UNDERFLOW)
        return 0.0; // underflow
    return tmp;
}*/