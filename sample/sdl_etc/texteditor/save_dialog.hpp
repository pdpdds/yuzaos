#pragma once
#include "base_text_buffer.hpp"
#include <functional.h>

class Screen;
class TextFile;

class SaveDialog: public BaseTextBuffer
{
	RTTI_DECLARATIONS(SaveDialog, BaseTextBuffer)
public:
  SaveDialog(Screen *screen, TextFile *textFile);
  eastl::function<void (SaveDialog *, TextFile *, const eastl::string &)> saveAs;
private:
  Screen *screen_;
  TextFile *textFile_;
  void scanDirectory();
  virtual eastl::wstring preInsert(Coord &cursor, const eastl::wstring &);
  virtual int preDelete(const Coord cursor, int = 1);
  virtual int preBackspace(Coord &cursor, int = 1);
};
