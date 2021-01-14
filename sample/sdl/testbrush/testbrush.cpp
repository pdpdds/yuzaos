

#include <SDL.h>
#include <cairo.h>

#include "GdiPlusFlat.h"
#include "testhelpers.h"

//GpStatus GdipCreateFromXDrawable_linux(Drawable d, Display* dpy, GpGraphics** graphics);

#define CHECK_GDIP_ST(st)	do { if(st != Ok) { g_print("got st: %d expected Ok", st); } } while (0)

extern GpStatus GdipCreateFromSDL(SDL_Surface * sdl_surface, GpGraphics * *graphics);
static void
win_init(SDL_Surface* sdl_surface)
{
	GpGraphics* gp;
	GpStatus st;
	GpImage* img;
	gunichar2* unis;
	//GdipCreateFromXDrawable_linux(win->win, win->dpy, &gp);
	GdipCreateFromSDL(sdl_surface, &gp);
	{
		GpPen* pen;
		GpSolidFill* brush;
		int a = 255;
		int r = 255;
		int g = 0;
		int b = 0;

		GdipCreatePen1((guint32)a << 24 | r << 16 | g << 8 | b,
			10, UnitPixel, &pen);

		GdipDrawRectangle(gp, pen, 10, 10, 60, 60);
		GdipDrawLine(gp, pen, 0, 0, 100, 100);

		GdipCreateSolidFill((guint32)a << 24 | r << 16 | g << 8 | b, &brush);

		printf("%d\n", GdipFillEllipse(gp, (GpBrush*)brush, 40, 40, 50, 75));


		//		return;
	}
	



	/*unis = createWchar("test.jpg");
	st = GdipLoadImageFromFile(unis, &img);
	CHECK_GDIP_ST(st);
	st = GdipDrawImage(gp, img, 0, 0);
	CHECK_GDIP_ST(st);
	freeWchar(unis);
	GdipDisposeImage(img);
	img = NULL;

	printf("jpg drawn \n");

	unis = createWchar("test.tif");
	st = GdipLoadImageFromFile(unis, &img);
	CHECK_GDIP_ST(st);
	st = GdipDrawImage(gp, img, 100, 0);
	CHECK_GDIP_ST(st);
	freeWchar(unis);
	GdipDisposeImage(img);
	img = NULL;

	printf("tif drawn \n");

	unis = createWchar("test.gif");
	st = GdipLoadImageFromFile(unis, &img);
	CHECK_GDIP_ST(st);
	st = GdipDrawImage(gp, img, 200, 0);
	CHECK_GDIP_ST(st);
	freeWchar(unis);
	GdipDisposeImage(img);
	img = NULL;

	printf("gif drawn \n");

	unis = createWchar("test.png");
	st = GdipLoadImageFromFile(unis, &img);
	CHECK_GDIP_ST(st);
	st = GdipDrawImage(gp, img, 0, 100);
	CHECK_GDIP_ST(st);
	freeWchar(unis);
	GdipDisposeImage(img);
	img = NULL;

	printf("png drawn \n");

	unis = createWchar("test.bmp");
	st = GdipLoadImageFromFile(unis, &img);
	CHECK_GDIP_ST(st);
	st = GdipDrawImage(gp, img, 200, 100);
	CHECK_GDIP_ST(st);
	freeWchar(unis);
	GdipDisposeImage(img);
	img = NULL;

	printf("bmp drawn \n");*/

	GdipDeleteGraphics(gp);
}

int
main(int argc, char* argv[])
{
	
	STARTUP;

	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window* window = SDL_CreateWindow("An SDL2 window",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		640,
		480,
		SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);

	SDL_Renderer* renderer = SDL_CreateRenderer(window,
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

	SDL_Surface* sdl_surface = SDL_CreateRGBSurface(0,
		renderer_width,
		renderer_height,
		32,
		0x00ff0000,
		0x0000ff00,
		0x000000ff,
		0);

	printf("sdl_surface->w=%d\nsdl_surface->h=%d\nsdl_surface->pitch=%d\n", sdl_surface->w, sdl_surface->h, sdl_surface->pitch);
	printf("sdl_surface->format->format=%s\n", SDL_GetPixelFormatName(sdl_surface->format->format));

	cairo_surface_t* cairo_surface = cairo_image_surface_create_for_data((unsigned char*)sdl_surface->pixels,
		CAIRO_FORMAT_RGB24,
		sdl_surface->w,
		sdl_surface->h,
		sdl_surface->pitch);
	cairo_surface_set_device_scale(cairo_surface, cairo_x_multiplier, cairo_y_multiplier);


	//cairo_t* cairo_context = cairo_create(cairo_surface);

	
	win_init(sdl_surface);

	//win_draw(&win);

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
	//cairo_destroy(cairo_context);

	SDL_FreeSurface(sdl_surface);
	SDL_DestroyWindow(window);

	SDL_Quit();

	SHUTDOWN;
	return 0;
}

