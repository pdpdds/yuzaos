#pragma once
#include "color.hpp"
#include "coord.hpp"
#include <vector.h>
#include <set.h>
#include <eastl/string.h>
#include <map.h>

class BaseTextBuffer;

class CppHighlighter
{
public:
  CppHighlighter(BaseTextBuffer *);
  Color fgColor(int x, int y) const;
  Color bgColor(int x, int y) const;
  void update(const Coord &start, const Coord &end);
private:
  enum Type { Keyword, Ident, Comment, Macro, StringLiteral, Number, Other };
  eastl::vector<eastl::vector<Type> > types_;
  BaseTextBuffer *textBuffer_;
  eastl::set<eastl::string> keywords_;
  eastl::pair<Type, int> getToken(int &x, int &y);
  void moveForward(int &x, int &y) const;
  void moveBackward(int &x, int &y) const;
  bool outOfRange(int x, int y) const;
  wchar_t ch(int x, int y) const;
  mutable eastl::map<Type, Color> toFg_;
  mutable eastl::map<Type, Color> toBg_;
};
