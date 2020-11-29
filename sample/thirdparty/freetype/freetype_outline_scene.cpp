#include "freetype_outline_scene.hpp"

#include <eastl/algorithm.h>
#include <eastl/array.h>
#include <imgui.h>

#include "menu_scene.hpp"
#include "texture.hpp"
#include <utf8.h>
#include <eastl/vector.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

bool FreeTypeOutlineScene::Init(const Context &context) {
  auto error = FT_New_Face(context.ftLibrary, FONT, 0, &face);
  if (error)
    return false;
  eastl::fill(buffer.begin(), buffer.end(), 0);
  eastl::copy(eastl::begin(TEXT), eastl::end(TEXT), eastl::begin(buffer));
  return true;
}

void FreeTypeOutlineScene::Tick(const Context &context) {

  ImGui::Begin("Menu");
  ImGui::InputText("text", buffer.data(), bufferSize);
  ImGui::SliderInt("font size", &fontSize, 0, 128);

  float c[4]{color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, 1.0};

  ImGui::ColorPicker4("color", c, ImGuiColorEditFlags_RGB);
  color.r = c[0] * 255;
  color.g = c[1] * 255;
  color.b = c[2] * 255;

  bool quit = ImGui::Button("Back");

  ImGui::End();

  if (quit) {
    ChangeScene<MenuScene>(context);

    return;
  }

  FT_Set_Pixel_Sizes(face, 0, fontSize);

  DrawText(buffer, color, 300, 300, face, context);
}

void FreeTypeOutlineScene::Cleanup(const Context &context) {
  FT_Done_Face(face);
}

void FreeTypeOutlineScene::DrawSpansCallback(const int y, const int count,
                                             const FT_Span *const spans,
                                             void *const user) {
  SpanAdditionData *addl = static_cast<SpanAdditionData *>(user);
  for (int i = 0; i < count; ++i) {
    int x1 = spans[i].x + addl->dest.x;
    int y1 = addl->dest.y - y;
    int x2 = x1 + spans[i].len;
    int y2 = y1;

    SDL_SetRenderDrawBlendMode(addl->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(addl->renderer, addl->color.r, addl->color.g,
                           addl->color.b, spans[i].coverage);
    SDL_RenderDrawLine(addl->renderer, x1, y1, x2, y2);
  }
}

void FreeTypeOutlineScene::DrawText(const eastl::array<char, bufferSize> &text,
                                    const SDL_Color &color, const int &baseline,
                                    const int &x_start, const FT_Face &face,
                                    const Context &context) {

    eastl::vector<FT_ULong> charactors;
  auto end_it = utf8::find_invalid(text.begin(), text.end());
  utf8::utf8to16(text.begin(), end_it, eastl::back_inserter(charactors));

  int x = x_start;

  SpanAdditionData addl;
  addl.color = color;

  for (auto c : charactors) {
    FT_Load_Char(face, c, FT_LOAD_NO_BITMAP | FT_LOAD_NO_HINTING);
    addl.dest.x = x;
    addl.dest.y = baseline;
    addl.renderer = context.renderer;

    if (face->glyph->format == FT_GLYPH_FORMAT_OUTLINE) {
      FT_Raster_Params params;
      memset(&params, 0, sizeof(params));
      params.flags = FT_RASTER_FLAG_AA | FT_RASTER_FLAG_DIRECT;
      params.gray_spans = DrawSpansCallback;
      params.user = &addl;

      FT_Outline_Render(context.ftLibrary, &face->glyph->outline, &params);
    } // No fallback
    x += (face->glyph->metrics.horiAdvance >> 6);
  }
}
