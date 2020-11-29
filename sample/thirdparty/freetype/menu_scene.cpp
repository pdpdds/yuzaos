#include "menu_scene.hpp"

#include <imgui.h>

#include "freetype_harfbuzz_scene.hpp"
#include "freetype_scene.hpp"
#include "freetype_stroke_scene.hpp"
#include "freetype_outline_scene.hpp"

bool MenuScene::Init(const Context& context) { return true; }

void MenuScene::Tick(const Context& context) {
  ImGui::Begin("Menu");
  if (ImGui::Button("FreeType")) {
    ChangeScene<FreeTypeScene>(context);
  }

  if (ImGui::Button("FreeType + Outline Renderer")) {
    ChangeScene<FreeTypeOutlineScene>(context);
  }

  if (ImGui::Button("FreeType + Stroke")) {
    ChangeScene<FreeTypeStrokeScene>(context);
  }

  if (ImGui::Button("FreeType + Harfbuzz")) {
    ChangeScene<FreeTypeHarfbuzzScene>(context);
  }
  ImGui::End();
}

void MenuScene::Cleanup(const Context& context) {}