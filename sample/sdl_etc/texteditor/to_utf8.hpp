#pragma once
#include <eastl/string.h>

eastl::string toUtf8(const eastl::wstring &);
eastl::string toUtf8(const wchar_t *);
