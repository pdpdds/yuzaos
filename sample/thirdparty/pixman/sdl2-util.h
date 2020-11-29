#pragma once
#include <SDL.h>
#include <cairo.h>
#include <pixman.h>

pixman_image_t* convert_image(pixman_image_t* image);
bool render_pixman_image(cairo_t* cairo_context, pixman_image_t* pimage);

cairo_t* init_sdl(int width, int height);
bool loop_sdl();