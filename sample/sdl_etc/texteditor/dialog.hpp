#pragma once
#include "base_text_buffer.hpp"
#include <string.h>
#include <functional.h>

class Dialog: public BaseTextBuffer
{
	RTTI_DECLARATIONS(Dialog, BaseTextBuffer)
public:
  enum Answer { Yes = 1, No = 2, Cancel = 4};
  typedef unsigned Answers;
  Dialog(const eastl::wstring &message, Answers = Yes | No | Cancel);
  eastl::function<void (Answer)> result;
private:
	eastl::wstring message_;
  Answers answers_;
  virtual eastl::wstring preInsert(Coord &cursor, const eastl::wstring &value);
  virtual int preBackspace(Coord &cursor, int value);
};
