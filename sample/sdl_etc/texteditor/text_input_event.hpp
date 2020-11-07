#pragma once
#include <eastl/string.h>

class TextInputEvent
{
public:
  TextInputEvent(const eastl::wstring &);
  eastl::wstring text() const;
private:
	eastl::wstring text_;
};
