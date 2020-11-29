#include "freetype_stroke_scene.hpp"

#include <imgui.h>
#include <utf8.h>
#include <eastl/vector.h>

#include "menu_scene.hpp"
#include "texture.hpp"

static FT_Library library;

bool FreeTypeStrokeScene::Init(const Context& context) {
	FT_Init_FreeType(&library);
	auto error = FT_New_Face(library, FONT, 0, &face);
	if (error)
		return false;

	FT_Stroker_New(library, &stroker);

	eastl::fill(buffer.begin(), buffer.end(), 0);
	eastl::copy(eastl::begin(TEXT), eastl::end(TEXT), eastl::begin(buffer));

	return true;
}

void FreeTypeStrokeScene::Tick(const Context& context) {
	ImGui::Begin("Menu");

	ImGui::InputText("text", buffer.data(), bufferSize);

	ImGui::SliderInt("font size", &fontSize, 0, 128);

	float c[3]{ color.r / 255.0f, color.g / 255.0f, color.b / 255.0f };

	ImGui::ColorEdit3("color", c);
	color.r = c[0] * 255;
	color.g = c[1] * 255;
	color.b = c[2] * 255;

	ImGui::SliderInt("border size", &borderSize, 1, 10);

	c[0] = border_color.r / 255.0f;
	c[1] = border_color.g / 255.0f;
	c[2] = border_color.b / 255.0f;

	ImGui::ColorEdit3("border color", c, ImGuiColorEditFlags_RGB);
	border_color.r = c[0] * 255;
	border_color.g = c[1] * 255;
	border_color.b = c[2] * 255;

	bool quit = ImGui::Button("Back");

	ImGui::End();

	if (quit) {
		ChangeScene<MenuScene>(context);

		return;
	}

	FT_Set_Pixel_Sizes(face, 0, fontSize);
	FT_Stroker_Set(stroker, borderSize << 6, FT_STROKER_LINECAP_ROUND,
		FT_STROKER_LINEJOIN_ROUND, 0);

	DrawText(buffer, color, 300, 300, face, stroker, border_color,
		context.renderer);
}

void FreeTypeStrokeScene::Cleanup(const Context& context) {
	FT_Stroker_Done(stroker);
	FT_Done_Face(face);
}

void FreeTypeStrokeScene::DrawGlyph(FT_Glyph glyph, const SDL_Color& color,
	int& x, const int& baseline,
	SDL_Renderer* renderer) {
	FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, NULL, 0);

	FT_BitmapGlyph glyph_bitmap = (FT_BitmapGlyph)glyph;

	if (glyph_bitmap->bitmap.width != 0 && glyph_bitmap->bitmap.rows != 0) {
		SDL_Texture* glyph_texture =
			CreateTextureFromFT_Bitmap(renderer, glyph_bitmap->bitmap, color);

		SDL_Rect dest;
		SDL_QueryTexture(glyph_texture, NULL, NULL, &dest.w, &dest.h);
		dest.x = x + (glyph_bitmap->left);
		dest.y = baseline - (glyph_bitmap->top);

		SDL_SetTextureBlendMode(glyph_texture, SDL_BLENDMODE_BLEND);

		SDL_RenderCopy(renderer, glyph_texture, NULL, &dest);
		SDL_DestroyTexture(glyph_texture);
	}
	x += (glyph_bitmap->root.advance.x >> 16);
}

void FreeTypeStrokeScene::DrawText(const eastl::array<char, bufferSize>& text,
	const SDL_Color& color, const int& baseline,
	const int& x_start, const FT_Face& face,
	const FT_Stroker& stroker,
	const SDL_Color& border_color,
	SDL_Renderer* renderer) {
	eastl::vector<FT_ULong> charactors;
	auto end_it = utf8::find_invalid(text.begin(), text.end());
	utf8::utf8to16(text.begin(), end_it, eastl::back_inserter(charactors));

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	// Pass#1 Border
	int x = x_start;
	for (auto c : charactors) {
		FT_Load_Char(face, c, FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING);

		FT_Glyph glyph;
		FT_Get_Glyph(face->glyph, &glyph);
		FT_Glyph_StrokeBorder(&glyph, stroker, false, true);

		DrawGlyph(glyph, border_color, x, baseline, renderer);
		FT_Done_Glyph(glyph);
	}

	// Pass#2 Glyph
	x = x_start;
	for (auto c : charactors) {
		FT_Load_Char(face, c, FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING);

		FT_Glyph glyph;
		FT_Get_Glyph(face->glyph, &glyph);

		DrawGlyph(glyph, color, x, baseline, renderer);
		FT_Done_Glyph(glyph);
	}
}
