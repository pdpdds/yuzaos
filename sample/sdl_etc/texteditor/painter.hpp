#pragma once
#include "color.hpp"
#include <SDL.h>
#include <SDL_ttf.h>
#include <eastl/string.h>
#include <map.h>
#include <tuple.h>
#include <list.h>

class PaintDevice;

class Painter
{
public:
  Painter(PaintDevice *);
  ~Painter();
  void drawLine(int x1, int y1, int x2, int y2);
  void drawPoint(int x, int y);
  void drawRect(int x1, int y1, int x2, int y2);
  void setColor(Color);
  void renderGlyph(wchar_t ch, int x, int y, Color fg, Color bg);
  int glyphWidth() const;
  int glyphHeight() const;
private:
  SDL_Renderer *renderer_;
  int width_;
  int height_;
  TTF_Font *font_;
  typedef eastl::tuple<wchar_t, Color, Color> GlyphCacheKey;
  typedef eastl::map<GlyphCacheKey, eastl::tuple<SDL_Texture *, int, int, eastl::list<GlyphCacheKey>::iterator> > GlyphCache;
  GlyphCache glyphCache_;
  eastl::list<GlyphCacheKey> glyphCacheAge_;
};
