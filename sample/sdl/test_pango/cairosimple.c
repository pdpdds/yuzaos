#include <math.h>
#include <pango/pangocairo.h>
/*
static void
draw_text (cairo_t *cr)
{
#define RADIUS 150
#define N_WORDS 10
#define FONT "Sans Bold 27"

  PangoLayout *layout;
  PangoFontDescription *desc;
  int i;

  
  cairo_translate (cr, RADIUS, RADIUS);

 
  layout = pango_cairo_create_layout (cr);

  pango_layout_set_text (layout, "Text", -1);
  desc = pango_font_description_from_string (FONT);
  pango_layout_set_font_description (layout, desc);
  pango_font_description_free (desc);

 
  for (i = 0; i < N_WORDS; i++)
    {
      int width, height;
      double angle = (360. * i) / N_WORDS;
      double red;

      cairo_save (cr);

    
      red   = (1 + cos ((angle - 60) * G_PI / 180.)) / 2;
      cairo_set_source_rgb (cr, red, 0, 1.0 - red);

      cairo_rotate (cr, angle * G_PI / 180.);

     
      pango_cairo_update_layout (cr, layout);

      pango_layout_get_size (layout, &width, &height);
      cairo_move_to (cr, - ((double)width / PANGO_SCALE) / 2, - RADIUS);
      pango_cairo_show_layout (cr, layout);

      cairo_restore (cr);
    }

  g_object_unref (layout);
}
#include <systemcall_impl.h>
int main(int argc, char **argv)
{
  cairo_t *cr;
  char *filename;
  cairo_status_t status;
  cairo_surface_t *surface;

  if (argc != 2)
    {
      g_printerr ("Usage: cairosimple OUTPUT_FILENAME\n");
      return 1;
    }

  filename = argv[1];

  surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
					2 * RADIUS, 2 * RADIUS);
  cr = cairo_create (surface);


  cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
  cairo_paint (cr);
  draw_text (cr);
  cairo_destroy (cr);

  status = cairo_surface_write_to_png (surface, filename);
  cairo_surface_destroy (surface);

  if (status != CAIRO_STATUS_SUCCESS)
    {
      g_printerr ("Could not save png to '%s'\n", filename);
      return 1;
    }

  return 0;
}
*/

#include <SDL.h>

#include <SDL_Pango.h>

int main(int argc, char** argv)
{
    int width = 640;
    int height = 200;
    int margin = 10;
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Surface* surface;
    SDL_Texture* background;
    Uint32 rmask, gmask, bmask, amask;
    SDLPango_Context* context;
    SDL_Color textcolor = { 0xa9, 0xf8, 0x69, 0xff };

    const char* const text =
        "<span font=\"50\">"
        "Hello world "
        "أهلاً بالعالم "
        "你好世界 "
        "γειά σου κόσμος "
        "Здравствулте мир "
        "</span>";

    (void)argc;
    (void)argv;

    /* Create window and renderer. */
    window = SDL_CreateWindow("Hello world!",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        width,
        height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

    context = SDLPango_CreateContext();

    SDLPango_SetColor(context, &textcolor);
    SDLPango_SetMinimumSize(context, width - margin, height - margin);

    SDLPango_SetMarkup(context, text, -1);

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

    surface = SDL_CreateRGBSurface(0, width, height, 32, rmask, gmask, bmask, amask);

    SDLPango_Draw(context, surface, margin, margin);

    background = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    SDL_SetRenderDrawColor(renderer, 0x83, 0x1c, 0xc5, 0xff);
    SDL_RenderClear(renderer);

    SDL_RenderCopy(renderer, background, NULL, NULL);

    for (;;) {
        SDL_Event event;

        if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
            break;

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(background);
    SDLPango_FreeContext(context);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}