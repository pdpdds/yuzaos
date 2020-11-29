#ifndef FREETYPE_STROKE_SCENE_HPP
#define FREETYPE_STROKE_SCENE_HPP

#include <SDL.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
#include <eastl/array.h>
#include <eastl/string.h>

#include "scene.hpp"

class FreeTypeStrokeScene : public Scene {
public:
  virtual bool Init(const Context &context);
  virtual void Tick(const Context &context);
  virtual void Cleanup(const Context &context);

private:
  static constexpr size_t bufferSize = 256;

  static void DrawText(const eastl::array<char, bufferSize> &text,
                       const SDL_Color &color, const int &baseline,
                       const int &x_start, const FT_Face &face,
                       const FT_Stroker &stroker, const SDL_Color &border_color,
                       SDL_Renderer *renderer);

  static void FreeTypeStrokeScene::DrawGlyph(FT_Glyph glyph,
                                             const SDL_Color &color, int &x,
                                             const int &baseline,
                                             SDL_Renderer *renderer);

  const eastl::string TEXT = "Test";
  static constexpr auto FONT = "Sarabun-Regular.ttf";

  FT_Face face;
  FT_Stroker stroker;

  int fontSize = 64;
  SDL_Color color = {0, 0, 0, 255};

  int borderSize = 2;
  SDL_Color border_color = {0xEE, 0x10, 0xCC};
  eastl::array<char, bufferSize> buffer;
};

#endif
