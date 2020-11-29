#include <SDL.h>
#include <imgui.h>

#include "imgui_impl_sdl.h"
#include <imgui_sdl.h>
#include "menu_scene.hpp"
#include "scene.hpp"
#include "context.hpp"

constexpr int WIDTH = 640;
constexpr int HEIGHT = 480;

void* operator new[](size_t size, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
	return new uint8_t[size];
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return new uint8_t[size];
}

int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window* window =
		SDL_CreateWindow("Test Window", SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, 0);
	SDL_Renderer* renderer;
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF("Sarabun-Regular.ttf", 20.0f, NULL,
		io.Fonts->GetGlyphRangesThai());

	ImGuiSDL::Initialize(renderer, WIDTH, HEIGHT);
	ImGui_ImplSDL2_Init(window);

	Context c{ renderer, nullptr };
	FT_Init_FreeType(&c.ftLibrary);

	bool run = true;
	while (run)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event)) 
		{
			ImGui_ImplSDL2_ProcessEvent(&event);
			if (event.type == SDL_QUIT)
			{
				run = false;
				break;
			}
		}

		SDL_SetRenderDrawColor(renderer, 0x50, 0x82, 0xaa, 0xff);
		SDL_RenderClear(renderer);

		ImGui::NewFrame();

		Scene::TickCurrent(c);

		ImGui_ImplSDL2_UpdateMousePosAndButtons();
		ImGui_ImplSDL2_UpdateMouseCursor();
		ImGui_ImplSDL2_UpdateGamepads();

		ImGui::Render();
		ImGuiSDL::Render(ImGui::GetDrawData());

		SDL_RenderPresent(renderer);
	}

	ImGui_ImplSDL2_Shutdown();
	ImGuiSDL::Deinitialize();
	ImGui::DestroyContext();

	FT_Done_FreeType(c.ftLibrary);

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}