#pragma once
#include "base_text_buffer.hpp"

class Screen;

class IsearchBuffer: public BaseTextBuffer
{
	RTTI_DECLARATIONS(IsearchBuffer, BaseTextBuffer)
public:
  IsearchBuffer(Screen *);
  void findNext();
private:
  Screen *screen_;
  eastl::wstring searchString_;
  Coord initialCursor_;
  bool search();
  virtual void postInsert(Coord &cursor, const eastl::wstring &);
  virtual int preBackspace(Coord &cursor, int = 1);
  virtual void postBackspace(Coord &cursor, int = 1);
};
