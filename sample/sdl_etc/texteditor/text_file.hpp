#pragma once
#include "base_text_buffer.hpp"

class TextFile: public BaseTextBuffer
{
	RTTI_DECLARATIONS(TextFile, BaseTextBuffer)
public:
  TextFile(const eastl::string &fileName = "");
  eastl::string fileName() const;
  void save();
  void saveAs(const eastl::string &fileName);
private:
	eastl::string fileName_;
	eastl::wstring preInsert(Coord &cursor, const eastl::wstring &);
};
