#include "dialog.hpp"
#include <sstream>

Dialog::Dialog(const eastl::wstring &message, Answers answers):
  message_(message),
  answers_(answers)
{
    //20201105
  /*  eastl::ostringstream tmp;
  tmp << message;
  bool first = true;
  if ((answers_ & Yes) != 0)
  {
    if (first)
      tmp << L" (";
    else
      tmp << L"/";
    tmp << L"y";
    first = false;
  }
  if ((answers_ & No) != 0)
  {
    if (first)
      tmp << L"(";
    else
      tmp << L"/";
    tmp << L"n";
    first = false;
  }
  if ((answers_ & Cancel) != 0)
  {
    if (first)
      tmp << L"(";
    else
      tmp << L"/";
    tmp << L"c";
    first = false;
  }
  if (!first)
    tmp << L"): ";
  buffer_.push_back(tmp.str());*/
}

eastl::wstring Dialog::preInsert(Coord &, const eastl::wstring &value)
{
  if ((answers_ & Yes) != 0 && value.find(L'y') != eastl::wstring::npos)
    result(Yes);
  if ((answers_ & No) != 0 && value.find(L'n') != eastl::wstring::npos)
    result(No);
  if ((answers_ & Cancel) != 0 && value.find(L'c') != eastl::wstring::npos)
    result(Cancel);
  return L"";
}

int Dialog::preBackspace(Coord &, int)
{
  return 0;
}
