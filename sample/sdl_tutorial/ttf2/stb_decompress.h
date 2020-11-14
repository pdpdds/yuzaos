#pragma once
#include <SDL.h>

SDL_RWops* GetFontDataFromMemoryCompressedTTF(const char* compressed_ttf, unsigned int compressed_ttf_size);
SDL_RWops* GetFontDataFromMemoryCompressedBase85TTF(const char* compressed_ttf_data_base85);