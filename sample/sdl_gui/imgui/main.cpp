#include <SDL.h>
#include "imgui_sdl.h"
#include "ImGuiFileDialog.h"

ImVec4 clear_color = ImColor(114, 144, 154);
bool show_test_window = true;
bool show_another_window = false;
bool show_demo_window = false;

void ShowDemo()
{
	{
		static float f = 0.0f;
		ImGui::Text("Hello, world!");                           // Some text (you can use a format string too)
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float as a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats as a color
		if (ImGui::Button("Demo Window"))                       // Use buttons to toggle our bools. We could use Checkbox() as well.
			show_demo_window ^= 1;
		if (ImGui::Button("Another Window"))
			show_another_window ^= 1;
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		static bool openFileDialog = false;

		if (ImGui::Button("Open File Dialog"))
		{
			openFileDialog = true;
		}

		static std::string filePathName = "";
		static std::string path = "";
		static std::string fileName = "";
		static std::string filter = "";

		if (openFileDialog)
		{
			if (ImGuiFileDialog::Instance()->FileDialog("Choose File", ".dll\0.h\0.hpp\0\0", ".", ""))
			{
				if (ImGuiFileDialog::Instance()->IsOk == true)
				{
					filePathName = ImGuiFileDialog::Instance()->GetFilepathName();
					path = ImGuiFileDialog::Instance()->GetCurrentPath();
					fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();
					filter = ImGuiFileDialog::Instance()->GetCurrentFilter();
				}
				else
				{
					filePathName = "";
					path = "";
					fileName = "";
					filter = "";
				}
				openFileDialog = false;
			}
		}

		if (filePathName.size() > 0) ImGui::Text("Choosed File Path Name : %s", filePathName.c_str());
		if (path.size() > 0) ImGui::Text("Choosed Path Name : %s", path.c_str());
		if (fileName.size() > 0) ImGui::Text("Choosed File Name : %s", fileName.c_str());
		if (filter.size() > 0) ImGui::Text("Choosed Filter : %s", filter.c_str());
	}

	// 2. Show another simple window. In most cases you will use an explicit Begin/End pair to name the window.
	if (show_another_window)
	{
		ImGui::Begin("Another Window", &show_another_window);
		ImGui::Text("Hello from another window!");
		ImGui::End();
	}

	// 3. Show the ImGui demo window. Most of the sample code is in ImGui::ShowDemoWindow().
	if (show_demo_window)
	{
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver); // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
		ImGui::ShowDemoWindow(&show_demo_window);
	}
}

int main(int argc, char** argv)
{
	//SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window* window = SDL_CreateWindow("SDL2 ImGui Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_RESIZABLE);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

	ImGui::CreateContext();
	ImGuiSDL::Initialize(renderer, 800, 600);

	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, 100, 100);
	{
		SDL_SetRenderTarget(renderer, texture);
		SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
		SDL_RenderClear(renderer);
		SDL_SetRenderTarget(renderer, nullptr);
	}

	bool run = true;
	while (run)
	{
		ImGuiIO& io = ImGui::GetIO();

		int wheel = 0;

		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT) run = false;
			else if (e.type == SDL_WINDOWEVENT)
			{
				if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
				{
					io.DisplaySize.x = static_cast<float>(e.window.data1);
					io.DisplaySize.y = static_cast<float>(e.window.data2);
				}
			}
			else if (e.type == SDL_MOUSEWHEEL)
			{
				wheel = e.wheel.y;
			}
		}

		int mouseX, mouseY;
		const int buttons = SDL_GetMouseState(&mouseX, &mouseY);

		// Setup low-level inputs (e.g. on Win32, GetKeyboardState(), or write to those fields from your Windows message loop handlers, etc.)

		io.DeltaTime = 1.0f / 60.0f;
		io.MousePos = ImVec2(static_cast<float>(mouseX), static_cast<float>(mouseY));
		io.MouseDown[0] = buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
		io.MouseDown[1] = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);
		io.MouseWheel = static_cast<float>(wheel);

		ImGui::NewFrame();

		//ImGui::ShowDemoWindow();

		ShowDemo();

		ImGui::Begin("Image");
		ImGui::Image(texture, ImVec2(100, 100));
		ImGui::End();

		SDL_SetRenderDrawColor(renderer, 114, 144, 154, 255);
		SDL_RenderClear(renderer);

		ImGui::Render();
		ImGuiSDL::Render(ImGui::GetDrawData());

		SDL_RenderPresent(renderer);
	}

	ImGuiSDL::Deinitialize();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	ImGui::DestroyContext();

	return 0;
}