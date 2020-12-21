/*
** SDL Code Edit
**
** Copyright (C) 2018 Wang Renxin
**
** A code edit widget in plain SDL.
**
** For the latest info, see https://github.com/paladin-t/sdl_code_edit/
*/

#ifdef _MSC_VER
#	ifndef _CRT_SECURE_NO_WARNINGS
#		define _CRT_SECURE_NO_WARNINGS
#	endif /* _CRT_SECURE_NO_WARNINGS */
#endif /* _MSC_VER */

#define NOMINMAX
#include "TextEditor.h"
#include <SDL.h>
#include "SDL2_gfxPrimitives.h"
//#include <sstream>
#include <ostringstream>
#include "imgui_impl_sdl.h"
#include "imgui_sdl.h"
#include <fstream>
#include <ImGuiFileDialog.h>


#ifndef WINDOW_WIDTH
#	define WINDOW_WIDTH 640
#endif /* WINDOW_WIDTH */
#ifndef WINDOW_HEIGHT
#	define WINDOW_HEIGHT 480
#endif /* WINDOW_HEIGHT */

#ifndef WIDGET_BORDER_X
#	define WIDGET_BORDER_X 8
#endif /* WIDGET_BORDER_X */
#ifndef WIDGET_BORDER_Y
#	define WIDGET_BORDER_Y 20
#endif /* WIDGET_BORDER_Y */

#ifndef SCROLL_BAR_SIZE
#	define SCROLL_BAR_SIZE 8
#endif /* SCROLL_BAR_SIZE */

typedef jpcre2::select<char> jp;


void* operator new[](size_t size, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
	return new uint8_t[size];
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return new uint8_t[size];
}

template<typename T> T clamp(T v, T lo, T hi) {
	assert(lo <= hi);
	if (v < lo) v = lo;
	if (v > hi) v = hi;

	return v;
}

template<typename T> bool intersects(T px, T py, T x0, T y0, T x1, T y1) {
	return (px > eastl::min(x0, x1) && px < eastl::max(x0, x1)) && (py > eastl::min(y0, y1) && py < eastl::max(y0, y1));
}

#include <IOStream>
int main(int argc, char** argv)
{
	
	SDL_Window* window = SDL_CreateWindow("SDL2 ImGui Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_RESIZABLE);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

	ImGui::CreateContext();
	ImGuiSDL::Initialize(renderer, 800, 600);
	ImGui_ImplSDL2_Init(window);

	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, 100, 100);
	{
		SDL_SetRenderTarget(renderer, texture);
		SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
		SDL_RenderClear(renderer);
		SDL_SetRenderTarget(renderer, nullptr);
	}

	ImVec4 clear_col = ImColor(114, 144, 154);

	///////////////////////////////////////////////////////////////////////
	// TEXT EDITOR SAMPLE
	TextEditor editor;
	auto lang = TextEditor::LanguageDefinition::CPlusPlus();

	// set your own known preprocessor symbols...
	static const char* ppnames[] = { "NULL", "PM_REMOVE",
		"ZeroMemory", "DXGI_SWAP_EFFECT_DISCARD", "D3D_FEATURE_LEVEL", "D3D_DRIVER_TYPE_HARDWARE", "WINAPI","D3D11_SDK_VERSION", "assert" };
	// ... and their corresponding values
	static const char* ppvalues[] = {
		"#define NULL ((void*)0)",
		"#define PM_REMOVE (0x0001)",
		"Microsoft's own memory zapper function\n(which is a macro actually)\nvoid ZeroMemory(\n\t[in] PVOID  Destination,\n\t[in] SIZE_T Length\n); ",
		"enum DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_DISCARD = 0",
		"enum D3D_FEATURE_LEVEL",
		"enum D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE  = ( D3D_DRIVER_TYPE_UNKNOWN + 1 )",
		"#define WINAPI __stdcall",
		"#define D3D11_SDK_VERSION (7)",
		" #define assert(expression) (void)(                                                  \n"
		"    (!!(expression)) ||                                                              \n"
		"    (_wassert(_CRT_WIDE(#expression), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0) \n"
		" )"
	};

	for (int i = 0; i < sizeof(ppnames) / sizeof(ppnames[0]); ++i)
	{
		TextEditor::Identifier id;
		id.mDeclaration = ppvalues[i];
		lang.mPreprocIdentifiers.insert(eastl::make_pair(eastl::string(ppnames[i]), id));
	}

	// set your own identifiers
	static const char* identifiers[] = {
		"HWND", "HRESULT", "LPRESULT","D3D11_RENDER_TARGET_VIEW_DESC", "DXGI_SWAP_CHAIN_DESC","MSG","LRESULT","WPARAM", "LPARAM","UINT","LPVOID",
		"ID3D11Device", "ID3D11DeviceContext", "ID3D11Buffer", "ID3D11Buffer", "ID3D10Blob", "ID3D11VertexShader", "ID3D11InputLayout", "ID3D11Buffer",
		"ID3D10Blob", "ID3D11PixelShader", "ID3D11SamplerState", "ID3D11ShaderResourceView", "ID3D11RasterizerState", "ID3D11BlendState", "ID3D11DepthStencilState",
		"IDXGISwapChain", "ID3D11RenderTargetView", "ID3D11Texture2D", "TextEditor" };
	static const char* idecls[] =
	{
		"typedef HWND_* HWND", "typedef long HRESULT", "typedef long* LPRESULT", "struct D3D11_RENDER_TARGET_VIEW_DESC", "struct DXGI_SWAP_CHAIN_DESC",
		"typedef tagMSG MSG\n * Message structure","typedef LONG_PTR LRESULT","WPARAM", "LPARAM","UINT","LPVOID",
		"ID3D11Device", "ID3D11DeviceContext", "ID3D11Buffer", "ID3D11Buffer", "ID3D10Blob", "ID3D11VertexShader", "ID3D11InputLayout", "ID3D11Buffer",
		"ID3D10Blob", "ID3D11PixelShader", "ID3D11SamplerState", "ID3D11ShaderResourceView", "ID3D11RasterizerState", "ID3D11BlendState", "ID3D11DepthStencilState",
		"IDXGISwapChain", "ID3D11RenderTargetView", "ID3D11Texture2D", "class TextEditor" };
	for (int i = 0; i < sizeof(identifiers) / sizeof(identifiers[0]); ++i)
	{
		TextEditor::Identifier id;
		id.mDeclaration = eastl::string(idecls[i]);
		lang.mIdentifiers.insert(eastl::make_pair(eastl::string(identifiers[i]), id));
	}
	editor.SetLanguageDefinition(lang);
	//editor.SetPalette(TextEditor::GetLightPalette());

	// error markers
	TextEditor::ErrorMarkers markers;
	markers.insert(eastl::make_pair<int, eastl::string>(6, "Example error here:\nInclude file not found: \"TextEditor.h\""));
	markers.insert(eastl::make_pair<int, eastl::string>(41, "Another example error"));
	editor.SetErrorMarkers(markers);

	// "breakpoint" markers
	TextEditor::Breakpoints bpts;
	bpts.insert(24);
	bpts.insert(47);
	editor.SetBreakpoints(bpts);
	static bool openFileDialog = false;
	static const char* fileToEdit = "ImGuiColorTextEdit/InitCPU.cpp";
	
	FILE* fp;
	char temp[256];
	char* filename = "InitCPU.cpp";
	eastl::string str;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("Could not open file %s", filename);
		return 1;
	}
	while (fgets(temp, 256, fp) != NULL)
		str += temp;
	fclose(fp);
	editor.SetText(str);
	
	//static const char* fileToEdit = "ImGuiColorTextEdit/TextEditor.cpp";
	/*{
		eastl::ifstream t(fileToEdit);
		if (t.good())
		{
			eastl::string str((eastl::istreambuf_iterator<char>(t)), eastl::istreambuf_iterator<char>());
			editor.SetText(str);
		}
	}*/



	// Loops.
	Uint64 timestamp = 0;
	SDL_Event e;
	bool done = false;
	while (!done) {
		// Processes events.
		while (SDL_PollEvent(&e)) 
		{
			ImGui_ImplSDL2_ProcessEvent(&e);

			switch (e.type) {
			case SDL_QUIT: {
					done = true;
				}
				break;
			default:
				break;
			}
			//edit->onEvent(&e);
		}
		ImGui::NewFrame();

		auto cpos = editor.GetCursorPosition();
		ImGui::Begin("Text Editor Demo", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
		ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open"))
				{
					openFileDialog = true;
				}
				if (ImGui::MenuItem("Save"))
				{
					auto textToSave = editor.GetText();
					/// save text....
				}
				if (ImGui::MenuItem("Quit", "Alt-F4"))
					break;
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				bool ro = editor.IsReadOnly();
				if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
					editor.SetReadOnly(ro);
				ImGui::Separator();

				if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && editor.CanUndo()))
					editor.Undo();
				if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && editor.CanRedo()))
					editor.Redo();

				ImGui::Separator();

				if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.HasSelection()))
					editor.Copy();
				if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && editor.HasSelection()))
					editor.Cut();
				if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && editor.HasSelection()))
					editor.Delete();
				if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
					editor.Paste();

				ImGui::Separator();

				if (ImGui::MenuItem("Select all", nullptr, nullptr))
					editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor.GetTotalLines(), 0));

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::MenuItem("Dark palette"))
					editor.SetPalette(TextEditor::GetDarkPalette());
				if (ImGui::MenuItem("Light palette"))
					editor.SetPalette(TextEditor::GetLightPalette());
				if (ImGui::MenuItem("Retro blue palette"))
					editor.SetPalette(TextEditor::GetRetroBluePalette());
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
			editor.IsOverwrite() ? "Ovr" : "Ins",
			editor.CanUndo() ? "*" : " ",
			editor.GetLanguageDefinition().mName.c_str(), fileToEdit);

		editor.Render("TextEditor");

		static eastl::string filePathName = "";
		static eastl::string path = "";
		static eastl::string fileName = "";
		static eastl::string filter = "";

		if (openFileDialog)
		{
			if (ImGuiFileDialog::Instance()->FileDialog("Choose File", ".txt\0.h\0.hpp\0\0", ".", ""))
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


		ImGui::End();

		SDL_SetRenderDrawColor(renderer, 114, 144, 154, 255);
		SDL_RenderClear(renderer);

		ImGui_ImplSDL2_UpdateMousePosAndButtons();
		ImGui_ImplSDL2_UpdateMouseCursor();
		ImGui_ImplSDL2_UpdateGamepads();

		const Uint64 now = SDL_GetPerformanceCounter();
		if (timestamp == 0)
			timestamp = now;
		const Uint64 diff = now - timestamp;
		const double ddiff = (double)diff / SDL_GetPerformanceFrequency();
		const double rest = 1.0 / 60.0 - ddiff; // 60 FPS.
		timestamp = now;
		if (rest > 0)
			SDL_Delay((Uint32)(rest * 1000));


		ImGui::Render();
		ImGuiSDL::Render(ImGui::GetDrawData());
		SDL_RenderPresent(renderer);
	}

	

	//primitivePurge();
	/*if (cursorInput)
		SDL_FreeCursor(cursorInput);
	if (cursorArrow)
		SDL_FreeCursor(cursorArrow);*/
	if (renderer)
		SDL_DestroyRenderer(renderer);
	if (window)
		SDL_DestroyWindow(window);

	return 0;
}
