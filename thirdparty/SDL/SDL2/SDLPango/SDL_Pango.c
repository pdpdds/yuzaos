#include "SDL_Pango.h"

#include <pango/pango.h>
#include <pango/pangoft2.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H

/**
 * Default font and size.
 */
static const char * const FONT_DESCRIPTION = "Sans 24";

/**
 * SDLPango_Context object that holds the required information SDLPango needs
 * across calls.
 */
struct __SDLPango_Context {
  PangoFontMap* fontmap;
  PangoContext* context;
  PangoLayout* layout;
  PangoFontDescription* fontdesc;
  SDL_Color color;
};

/* initialization/finalization ************************************************/

/*!
  Initialize the Glib and Pango API.
  This must be called before using other functions in this library,
  excepting SDLPango_WasInit.
  SDL does not have to be initialized before this call.


  @return always 0.
*/
int
SDLPango_Init(void)
{
  return 0;
}

/*!
  Query the initialization status of the Glib and Pango API.
  You may, of course, use this before SDLPango_Init to avoid
  initializing twice in a row.

  @return zero when already initialized.
  non-zero when not initialized.
*/
int
SDLPango_WasInit(void)
{
  return 0;
}

/*!
  Create a context which contains Pango objects.

  @return A pointer to the context as a SDLPango_Context*.
*/
SDLPango_Context*
SDLPango_CreateContext(void)
{
  SDLPango_Context *context = g_malloc(sizeof(SDLPango_Context));

  context->fontmap = pango_ft2_font_map_new();
  context->context = pango_font_map_create_context(context->fontmap);
  context->layout = pango_layout_new(context->context);
  context->fontdesc = pango_font_description_from_string(FONT_DESCRIPTION);
  context->color.r = 0xff;
  context->color.g = 0xff;
  context->color.b = 0xff;
  context->color.a = 0xff;

  return context;
}

/*!
  Free a context.

  @param *context [i/o] Context to be free
*/
void
SDLPango_FreeContext(SDLPango_Context *context)
{
  pango_font_description_free(context->fontdesc);
  g_object_unref(context->layout);
  g_object_unref(context->context);
  g_object_unref(context->fontmap);
  g_free(context);
}

/* public methods *************************************************************/

/*!
  Specify default color.

  @param *context [i/o] Context
  @param *color [in] Text color
*/
void
SDLPango_SetColor(SDLPango_Context* context,
                  const SDL_Color* color)
{
  context->color.r = color->r;
  context->color.g = color->g;
  context->color.b = color->b;
  context->color.a = color->a;
}

/*!
  Get the default color.

  @return Default color
*/
SDL_Color
SDLPango_GetColor(SDLPango_Context* context)
{
  return context->color;
}

/*!
  Specify minimum size of drawing rect.

  @param *context [i/o] Context
  @param width [in] Width. -1 means no wrapping mode.
  @param height [in] Height. zero/minus value means non-specified.
*/
void
SDLPango_SetMinimumSize(SDLPango_Context* context,
                        int width,
                        int height)
{
  pango_layout_set_width(context->layout, width > 0 ? width * PANGO_SCALE : -1);
  pango_layout_set_height(context->layout, height);
}

/*!
  Get layout width.

  @param *context [in] Context
  @return Width
*/
int
SDLPango_GetLayoutWidth(SDLPango_Context *context)
{
  int width;

  pango_layout_get_pixel_size(context->layout, &width, NULL);
  return width;
}

/*!
  Get layout height.

  @param *context [in] Context
  @return Height
*/
int
SDLPango_GetLayoutHeight(SDLPango_Context *context)
{
  int height;

  pango_layout_get_pixel_size(context->layout, NULL, &height);
  return height;
}

/*!
  Set markup text to context.
  Text must be utf-8.
  Markup format is same as pango.

  @param *context [i/o] Context
  @param *markup [in] Markup text
  @param length [in] Text length. -1 means NULL-terminated text.
*/
void
SDLPango_SetMarkup(SDLPango_Context *context,
                   const char *markup,
                   int length)
{
  pango_layout_set_markup(context->layout, markup, length);
  pango_layout_set_auto_dir(context->layout, TRUE);
  pango_layout_set_alignment(context->layout, PANGO_ALIGN_LEFT);
  pango_layout_set_font_description(context->layout, context->fontdesc);
}

/*!
  Draw text on a existing surface.

  @param *context [in] Context
  @param *surface [i/o] Surface to draw on it
  @param x [in] X of left-top of drawing area
  @param y [in] Y of left-top of drawing area
*/
void
SDLPango_Draw(SDLPango_Context *context,
              SDL_Surface *surface,
              int x,
              int y)
{
  int xindex;
  int yindex;
  FT_Bitmap bitmap;

  FT_Bitmap_Init(&bitmap);

  bitmap.width = surface->w;
  bitmap.rows = surface->h;
  bitmap.num_grays = 256;
  bitmap.pixel_mode = FT_PIXEL_MODE_GRAY;
  bitmap.pitch = bitmap.width;
  bitmap.buffer = g_malloc(bitmap.rows * bitmap.pitch);

  memset(bitmap.buffer, 0, bitmap.rows * bitmap.pitch);

  pango_ft2_render_layout(&bitmap, context->layout, x, y);

  SDL_LockSurface(surface);

  SDL_Color color = context->color;

  for(xindex = 0; xindex < surface->w ; xindex++) {
    for(yindex = 0; yindex < surface->h ; yindex++) {
      /* the grayscale value is used as alpha channel */
      Uint8 pixel = (bitmap.buffer)[yindex * surface->w + xindex];

      switch(surface->format->BytesPerPixel) {
        case 2:
          ((Uint16 *)surface->pixels)[yindex * surface->w + xindex]
            = (Uint16)SDL_MapRGBA(surface->format,
                color.r, color.g, color.b, pixel);
          break;
        case 4:
          ((Uint32 *)surface->pixels)[yindex * surface->w + xindex]
            = (Uint32)SDL_MapRGBA(surface->format,
                color.r, color.g, color.b, pixel);
          break;
        default:
          SDL_SetError("surface->format->BytesPerPixel is invalid value");
          break;
      }
    }
  }

  SDL_UnlockSurface(surface);

  g_free(bitmap.buffer);
}
