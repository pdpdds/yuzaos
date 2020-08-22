#include <stdio.h>
#include "sample.h"
#include <imgui.h>
#include "imgui_impl_sdl.h"
#include <SDL.h>

int samplef(void* framebuffer)
{
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Surface* screen;
	SDL_Texture* texture;

	// Setup SDL
	/*if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
	{
		//printf("Error: %s\n", SDL_GetError());
		return -1;
	}*/

	// Setup window
	int width = 1024;
	int height = 768;
	//윈도우와 렌더러를 생성
	//if (SDL_CreateWindowAndRenderer(width, height, 0, &window, &renderer) < 0)
//	{
		//std::cout << "SDL_CreateWindowAndRenderer Error: " << SDL_GetError() << std::endl;
		//return 0;
	//}

	
	window = SDL_CreateWindow("ImGui SDL2+SW rasterizer example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_RESIZABLE);
	renderer = SDL_CreateRenderer(window, -1, 0);
	screen = SDL_CreateRGBSurface(0, width, height, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);

	// Setup ImGui binding
	//screen->pixels = framebuffer;
	ImGui_ImplSdl_Init(window, screen);

	// Load Fonts
	// (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
	//ImGuiIO& io = ImGui::GetIO();
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("../../extra_fonts/Cousine-Regular.ttf", 15.0f);
	//io.Fonts->AddFontFromFileTTF("../../extra_fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyClean.ttf", 13.0f);
	//io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyTiny.ttf", 10.0f);
	//io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());

	bool show_test_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImColor(114, 144, 154);

	// Main loop
	bool done = false;
	while (!done)
	{
		SDL_Event event;

		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSdl_ProcessEvent(&event);
			if (event.type == SDL_QUIT)
				done = true;
		}
		
		ImGui_ImplSdl_NewFrame(window);

		// 1. Show a simple window
		// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
		{
			static float f = 0.0f;
			ImGui::Text("Hello, world!");
			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
			ImGui::ColorEdit3("clear color", (float*)&clear_color);
			if (ImGui::Button("Test Window")) show_test_window ^= 1;
			if (ImGui::Button("Another Window")) show_another_window ^= 1;
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		}

		// 2. Show another simple window, this time using an explicit Begin/End pair
		if (show_another_window)
		{
			ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("Another Window", &show_another_window);
			ImGui::Text("Hello");
			ImGui::End();
		}

		// 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
		/*if (show_test_window)
		{
			ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
			ImGui::ShowTestWindow();
		}*/

		// Rendering
		uint32_t col =
			((int)(clear_color.x * 255) << 16) |
			((int)(clear_color.y * 255) << 8) |
			((int)(clear_color.z * 255) << 0);
		uint32_t* pixels = (uint32_t*)screen->pixels;
		for (int y = 0; y < height; y++)
			for (int x = 0; x < width; x++)
				pixels[x + y * width] = col;
		ImGui::Render();

		memcpy(framebuffer, pixels, height*width * 4);
		//memcpy(framebuffer, screen->pixels, 1024 * 768 * 32);
		SDL_UpdateTexture(texture, NULL, screen->pixels, screen->pitch);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
	}

	// Cleanup
	ImGui_ImplSdl_Shutdown();
	SDL_DestroyTexture(texture);
	SDL_FreeSurface(screen);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}