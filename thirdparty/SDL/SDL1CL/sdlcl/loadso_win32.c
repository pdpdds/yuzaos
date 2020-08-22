/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2019 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "SDL2.h"
#include "loadso.h"
#include "main.h"

#define WIN_StringToUTF8(S) SDL_iconv_string("UTF-8", "ASCII", (char *)(S), (SDL_strlen(S)+1))
#define WIN_UTF8ToString(S) SDL_iconv_string("ASCII", "UTF-8", (char *)(S), SDL_strlen(S)+1)

extern char* SDLCALL SDL_iconv_string(const char* tocode, const char* fromcode, const char* inbuf, size_t inbytesleft);

DECLSPEC void* SDLCALL SDL_LoadObject(const char* sofile) {
	//LPTSTR tstr = WIN_UTF8ToString(sofile);
#ifdef __WINRT__
	/* WinRT only publically supports LoadPackagedLibrary() for loading .dll
	   files.  LoadLibrary() is a private API, and not available for apps
	   (that can be published to MS' Windows Store.)
	*/
	void* handle = (void*)LoadPackagedLibrary(tstr, 0);
#else
	void* handle = (void*)LoadLibrary(sofile);
#endif
	

	/* Generate an error message if all loads failed */
	/*if (handle == NULL) {
		char errbuf[512];
		SDL_strlcpy(errbuf, "Failed loading ", SDL_arraysize(errbuf));
		SDL_strlcat(errbuf, sofile, SDL_arraysize(errbuf));
		WIN_SetError(errbuf);
	}*/
	return handle;
}

DECLSPEC void* SDLCALL SDL_LoadFunction(void* handle, const char* name) {
	void* symbol = (void*)GetProcAddress((HMODULE)handle, name);
	/*if (symbol == NULL) {
		char errbuf[512];
		SDL_strlcpy(errbuf, "Failed loading ", SDL_arraysize(errbuf));
		SDL_strlcat(errbuf, name, SDL_arraysize(errbuf));
		WIN_SetError(errbuf);
	}*/
	return symbol;
}

DECLSPEC void SDLCALL SDL_UnloadObject(void* handle) {
	if (handle != NULL) {
		FreeLibrary((HMODULE)handle);
	}
}


