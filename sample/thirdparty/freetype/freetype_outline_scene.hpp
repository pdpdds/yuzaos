#ifndef FREETYPE_OUTLINE_SCENE_HPP
#define FREETYPE_OUTLINE_SCENE_HPP

#include <SDL.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "scene.hpp"
#include <eastl/array.h>
#include <eastl/string.h>

class FreeTypeOutlineScene : public Scene {
public:
  virtual bool Init(const Context &context);
  virtual void Tick(const Context &context);
  virtual void Cleanup(const Context &context);

private:
  static constexpr size_t bufferSize = 256;
  static void DrawText(const eastl::array<char, bufferSize> &text,
                       const SDL_Color &color, const int &baseline,
                       const int &x_start, const FT_Face &face,
                       const Context &context);

  struct SpanAdditionData {
    SDL_Renderer *renderer;
    SDL_Point dest;
    SDL_Color color;
  };

  static void DrawSpansCallback(const int y, const int count,
                                const FT_Span *const spans, void *const user);

  const eastl::string TEXT = "Test";
  const char *FONT = "Sarabun-Regular.ttf";
  FT_Face face;
  int fontSize = 64;
  SDL_Color color = {0, 0, 0, 255};

  eastl::array<char, bufferSize> buffer;
};

#endif
