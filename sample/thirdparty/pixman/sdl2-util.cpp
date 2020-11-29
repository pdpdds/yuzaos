#include <stdio.h>
#include "sdl2-util.h"

cairo_t* cairo_context;
SDL_Renderer* renderer;
SDL_Window* window;
SDL_Surface* sdl_surface;
cairo_surface_t* cairo_surface;

pixman_image_t* convert_image(pixman_image_t* image)
{
	int width, height;

	pixman_format_code_t format;
	pixman_image_t* copy;


	width = pixman_image_get_width(image);
	height = pixman_image_get_height(image);


	format = pixman_image_get_format(image);

	/* We always display the image as if it contains sRGB data. That
	 * means that no conversion should take place when the image
	 * has the a8r8g8b8_sRGB format.
	 */
	switch (format)
	{
	case PIXMAN_a8r8g8b8_sRGB:
	case PIXMAN_a8r8g8b8:
	case PIXMAN_x8r8g8b8:
		copy = pixman_image_ref(image);
		break;

	default:
		copy = pixman_image_create_bits(PIXMAN_a8r8g8b8,
			width, height, NULL, -1);
		pixman_image_composite32(PIXMAN_OP_SRC,
			image, NULL, copy,
			0, 0, 0, 0, 0, 0,
			width, height);
		break;
	}

	return copy;
}

cairo_t* init_sdl(int width, int height)
{

	SDL_Init(SDL_INIT_VIDEO);

	window = SDL_CreateWindow("An SDL2 window",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width,
		height,
		SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);

	renderer = SDL_CreateRenderer(window,
		-1,
		SDL_RENDERER_SOFTWARE);

	int window_width;
	int window_height;
	SDL_GetWindowSize(window, &window_width, &window_height);

	int renderer_width;
	int renderer_height;
	SDL_GetRendererOutputSize(renderer, &renderer_width, &renderer_height);

	int cairo_x_multiplier = renderer_width / window_width;
	int cairo_y_multiplier = renderer_height / window_height;

	sdl_surface = SDL_CreateRGBSurface(0,
		renderer_width,
		renderer_height,
		32,
		0x00ff0000,
		0x0000ff00,
		0x000000ff,
		0);

	printf("sdl_surface->w=%d\nsdl_surface->h=%d\nsdl_surface->pitch=%d\n", sdl_surface->w, sdl_surface->h, sdl_surface->pitch);
	printf("sdl_surface->format->format=%s\n", SDL_GetPixelFormatName(sdl_surface->format->format));

	cairo_surface = cairo_image_surface_create_for_data((unsigned char*)sdl_surface->pixels,
		CAIRO_FORMAT_RGB24,
		sdl_surface->w,
		sdl_surface->h,
		sdl_surface->pitch);

	cairo_surface_set_device_scale(cairo_surface, cairo_x_multiplier, cairo_y_multiplier);

	cairo_context = cairo_create(cairo_surface);

	return cairo_context;
}

bool loop_sdl()
{
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, sdl_surface);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);

	int done = 0;
	while (done == 0) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				done = 1;
				break;
			default:
				break;
			}
		}
	}

	cairo_surface_destroy(cairo_surface);
	cairo_destroy(cairo_context);

	SDL_FreeSurface(sdl_surface);
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}

bool render_pixman_image(cairo_t* cairo_context, pixman_image_t* pimage)
{
	int width = pixman_image_get_width(pimage);
	int height = pixman_image_get_height(pimage);

	int stride = pixman_image_get_stride(pimage);
	cairo_surface_t* cimage;
	cairo_format_t format;

	if (pixman_image_get_format(pimage) == PIXMAN_x8r8g8b8)
		format = CAIRO_FORMAT_RGB24;
	else
		format = CAIRO_FORMAT_ARGB32;

	//pixman이 생성한 이미지 데이터 버퍼를 사용해서 카이로용 서페이스를 생성
	cimage = cairo_image_surface_create_for_data(
		(uint8_t*)pixman_image_get_data(pimage),
		format, width, height, stride);

	cairo_rectangle(cairo_context, 0, 0, width, height);
	cairo_set_source_surface(cairo_context, cimage, 0, 0);
	cairo_fill(cairo_context);

	return true;
}