#include <stdio.h>
#include <SDL.h>
#include <cairo.h>

int main(void)
{
        SDL_Init(SDL_INIT_VIDEO);

        SDL_Window *window = SDL_CreateWindow("An SDL2 window",
                                  SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED,
                                  640,
                                  480,
                                  SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);

        SDL_Renderer *renderer = SDL_CreateRenderer(window,
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

        SDL_Surface *sdl_surface = SDL_CreateRGBSurface(0,
                                            renderer_width,
                                            renderer_height,
                                            32,
                                            0x00ff0000,
                                            0x0000ff00,
                                            0x000000ff,
                                            0);

        printf("sdl_surface->w=%d\nsdl_surface->h=%d\nsdl_surface->pitch=%d\n",sdl_surface->w,sdl_surface->h,sdl_surface->pitch);
        printf("sdl_surface->format->format=%s\n", SDL_GetPixelFormatName(sdl_surface->format->format));

        cairo_surface_t *cairo_surface = cairo_image_surface_create_for_data((unsigned char *)sdl_surface->pixels,
                                                                             CAIRO_FORMAT_RGB24,
                                                                             sdl_surface->w,
                                                                             sdl_surface->h,
                                                                             sdl_surface->pitch);

        cairo_surface_set_device_scale(cairo_surface, cairo_x_multiplier, cairo_y_multiplier);

        cairo_t *cairo_context = cairo_create(cairo_surface);

        // SDL_FillRect(sdl_surface, NULL, SDL_MapRGB(sdl_surface->format, 255, 255, 255));

        cairo_set_source_rgba(cairo_context, 1, 1, 1, 1.0);
        cairo_rectangle(cairo_context, 0, 0, 640, 480);
        cairo_fill(cairo_context);

        double xc = 320.0;
        double yc = 240.0;
        double radius = 200.0;
        double angle1 = 45.0  * (M_PI/180.0);
        double angle2 = 180.0 * (M_PI/180.0);

        cairo_set_source_rgba(cairo_context, 0, 0, 0, 1.0);
        cairo_set_line_width(cairo_context, 10.0);
        cairo_arc(cairo_context, xc, yc, radius, angle1, angle2);
        cairo_stroke(cairo_context);

        cairo_set_source_rgba(cairo_context, 1, 0.2, 0.2, 0.6);
        cairo_set_line_width(cairo_context, 6.0);

        cairo_arc(cairo_context, xc, yc, 10.0, 0, 2*M_PI);
        cairo_fill(cairo_context);

        cairo_arc(cairo_context, xc, yc, radius, angle1, angle1);
        cairo_line_to(cairo_context, xc, yc);
        cairo_arc(cairo_context, xc, yc, radius, angle2, angle2);
        cairo_line_to(cairo_context, xc, yc);
        cairo_stroke(cairo_context);

        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, sdl_surface);
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
