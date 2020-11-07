#pragma once
#include "base_text_buffer.hpp"
#include <functional.h>

class Screen;



class OpenDialog: public BaseTextBuffer
{
	RTTI_DECLARATIONS(OpenDialog, BaseTextBuffer)
public:
  OpenDialog(Screen *screen);
  eastl::function<void (OpenDialog *, const eastl::string &)> openFile;
private:
  Screen *screen_;
  void scanDirectory();
  virtual void postInsert(Coord &cursor, const eastl::wstring &);
};
