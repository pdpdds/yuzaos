#include "texture.hpp"

SDL_Texture *CreateTextureFromFT_Bitmap(SDL_Renderer *renderer,
                                        const FT_Bitmap &bitmap,
                                        const SDL_Color &color) {
  SDL_Texture *output =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                        SDL_TEXTUREACCESS_STREAMING, bitmap.width, bitmap.rows);

  if (output == nullptr) {
    return output;
  }

  void *buffer;
  int pitch;
  SDL_LockTexture(output, NULL, &buffer, &pitch);

  unsigned char *src_pixels = bitmap.buffer;
  unsigned int *target_pixels = reinterpret_cast<unsigned int *>(buffer);

  SDL_PixelFormat *pixel_format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);

  for (int y = 0; y < bitmap.rows; y++) {
    for (int x = 0; x < bitmap.width; x++) {
      int index = (y * bitmap.width) + x;

      unsigned int alpha = src_pixels[index];
      unsigned int pixel_value =
          SDL_MapRGBA(pixel_format, color.r, color.g, color.b, alpha);

      target_pixels[index] = pixel_value;
    }
  }

  SDL_FreeFormat(pixel_format);
  SDL_UnlockTexture(output);

  return output;
}