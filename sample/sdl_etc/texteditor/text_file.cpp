#include "text_file.hpp"
#include "to_utf16.hpp"
#include "to_utf8.hpp"
#include "cpp_highlighter.hpp"
#include "full_file_name.hpp"
#include <fstream>
#include <algorithm.h>

static eastl::string baseName(const eastl::string &fileName)
{
  auto p = fileName.rfind('/');
  if (p != eastl::string::npos)
    return eastl::string{ begin(fileName) + p + 1, end(fileName) };
  else
    return fileName;
}

static bool isCpp(const eastl::string &fileName)
{
  auto p = fileName.rfind(".");
  eastl::string ext;
  if (p != eastl::string::npos)
    ext = fileName.substr(p);
  static eastl::string exts[] = {
    ".c",
    ".cpp",
    ".C",
    ".cc",
    ".c++",
    ".h",
    ".H",
    ".hpp",
    ".h++"
  };
  return eastl::find(eastl::begin(exts), eastl::end(exts), ext) != eastl::end(exts);
}

TextFile::TextFile(const eastl::string &fileName):
  fileName_(getFullFileName(fileName))
{
  if (!fileName.empty())
    setName(toUtf16(baseName(fileName)));
  else
    setName(L"Untitled");
        
  eastl::ifstream f;
  f = fileName_.c_str();
  if (f.is_open())
    while (!f.eof())
    {
        eastl::string line;
        getline(f, line);
      buffer_.push_back(toUtf16(line));
    }
  else
    buffer_.push_back(L"");
  if (isCpp(fileName))
    highlighter_ = new CppHighlighter(this);
}

eastl::string TextFile::fileName() const
{
  return fileName_;
}

void TextFile::save()
{
  if (!fileName_.empty())
    saveAs(fileName_);
}

void TextFile::saveAs(const eastl::string &fileName)
{
  fileName_ = fileName;
  setName(toUtf16(baseName(fileName)));
  eastl::ofstream f;
  f = fileName_.c_str();
  bool first = true;
  for (const auto &l: buffer_)
  {
    if (first)
      first = false;
    else
      f << endl;
    f << toUtf8(l).c_str();
  }
  clearModified();
}


eastl::wstring TextFile::preInsert(Coord &cursor, const eastl::wstring &value)
{
  if (value.size() != 1 || value[0] != '\n')
    return value;
  else
  {
    auto &line = (*this)[cursor.y];
    eastl::wstring spaces;
    int c = 0;
    for (auto ch: line)
      if (ch == L' ')
        ++c;
      else
        break;
    for (size_t x = cursor.x; x < line.size() && line[x] == ' '; ++x, --c);
    for (int i = 0; i < c; ++i)
      spaces += L' ';
    return L'\n' + spaces;
  }
}
