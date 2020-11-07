#pragma once
#include "coord.hpp"
#include "cpp_highlighter.hpp"
#include "undo_stack.hpp"
#include <string.h>
#include <vector.h>

#include "RTTI.h"

class Screen;

class BaseTextBuffer : public Common::RTTI
{
public:
  BaseTextBuffer();
  virtual ~BaseTextBuffer();
  const eastl::wstring &operator[](int line) const;
  eastl::wstring &operator[](int line);
  int size() const;
  void undo(Coord &cursor);
  void redo(Coord &cursor);
  bool canUndo() const;
  bool canRedo() const;
  bool isModified() const;
  void clearModified();
  void render(Screen *) const;
  void insert(Coord &cursor, const eastl::wstring &);
  void del(Coord &cursor, int = 1);
  void backspace(Coord &cursor, int = 1);
  bool isReadOnly() const;
  void setReadOnly(bool);
  eastl::wstring name() const;
  void setName(const eastl::wstring &);
  Coord cursor() const;
  void setCursor(Coord);
protected:
  eastl::vector<eastl::wstring> buffer_;
  bool isReadOnly_;
  UndoStack undoStack_;
  eastl::wstring name_;
  Coord cursor_;
  CppHighlighter *highlighter_;
  virtual eastl::wstring preInsert(Coord &cursor, const eastl::wstring &);
  virtual void postInsert(Coord &cursor, const eastl::wstring &);
  virtual int preDel(Coord &cursor, int = 1);
  virtual void postDel(Coord &cursor, int = 1);
  virtual int preBackspace(Coord &cursor, int = 1);
  virtual void postBackspace(Coord &cursor, int = 1);
private:
  void internalInsert(Coord &cursor, const eastl::wstring &);
  eastl::wstring internalDelete(const Coord cursor, int = 1);
  eastl::wstring internalBackspace(Coord &cursor, int = 1);
};
