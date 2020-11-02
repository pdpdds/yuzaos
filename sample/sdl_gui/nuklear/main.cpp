#include <SystemCall_Impl.h>
#include "windef.h"
#include "string.h"
#include "memory.h"
#include <SDL.h>

//critical
//아래 함수 링크 에러를 막기 위해 sin, cos, Clfmod를 sinf, cosf, Clfmod로 변경
/*extern "C" void __cdecl _CIfmod()
{

}*/
/*extern "C" void __cdecl _libm_sse2_cos_precise()
{

}*/

/*extern "C" void __cdecl _libm_sse2_sin_precise()
{

}*/

#define RAWFB_RGBX_8888
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_IMPLEMENTATION
#define NK_XLIBSHM_IMPLEMENTATION
#define NK_RAWFB_IMPLEMENTATION
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_SOFTWARE_FONT
#define NK_IMPLEMENTATION
#define INCLUDE_CALCULATOR

#include "nuklear.h"
#include "nuklear_rawfb.h"
#include "sprintf.h"

#ifdef INCLUDE_STYLE
#include "../style.c"
#endif
#ifdef INCLUDE_CALCULATOR
#include "../calculator.c"
#endif
#ifdef INCLUDE_OVERVIEW
#include "../overview.c"
#endif
#ifdef INCLUDE_NODE_EDITOR
#include "./node_editor.c"
#endif

struct nk_font *font = 0;

/* Platform */
SDL_Window *win;
SDL_Renderer* renderer;
SDL_Surface* screen;
SDL_Texture* texture;

NK_API int nk_sdl_handle_event(SDL_Event *evt, nk_context* sdl_ctx)
{
	struct nk_context *ctx = sdl_ctx;

	/* optional grabbing behavior */
	if (ctx->input.mouse.grab) {
		SDL_SetRelativeMouseMode(SDL_TRUE);
		ctx->input.mouse.grab = 0;
	}
	else if (ctx->input.mouse.ungrab) {
		int x = (int)ctx->input.mouse.prev.x, y = (int)ctx->input.mouse.prev.y;
		SDL_SetRelativeMouseMode(SDL_FALSE);
		SDL_WarpMouseInWindow(win, x, y);
		ctx->input.mouse.ungrab = 0;
	}
	if (evt->type == SDL_KEYUP || evt->type == SDL_KEYDOWN) {
		/* key events */
		int down = evt->type == SDL_KEYDOWN;
		const Uint8* state = SDL_GetKeyboardState(0);
		SDL_Keycode sym = evt->key.keysym.sym;
		if (sym == SDLK_RSHIFT || sym == SDLK_LSHIFT)
			nk_input_key(ctx, NK_KEY_SHIFT, down);
		else if (sym == SDLK_DELETE)
			nk_input_key(ctx, NK_KEY_DEL, down);
		else if (sym == SDLK_RETURN)
			nk_input_key(ctx, NK_KEY_ENTER, down);
		else if (sym == SDLK_TAB)
			nk_input_key(ctx, NK_KEY_TAB, down);
		else if (sym == SDLK_BACKSPACE)
			nk_input_key(ctx, NK_KEY_BACKSPACE, down);
		else if (sym == SDLK_HOME) {
			nk_input_key(ctx, NK_KEY_TEXT_START, down);
			nk_input_key(ctx, NK_KEY_SCROLL_START, down);
		}
		else if (sym == SDLK_END) {
			nk_input_key(ctx, NK_KEY_TEXT_END, down);
			nk_input_key(ctx, NK_KEY_SCROLL_END, down);
		}
		else if (sym == SDLK_PAGEDOWN) {
			nk_input_key(ctx, NK_KEY_SCROLL_DOWN, down);
		}
		else if (sym == SDLK_PAGEUP) {
			nk_input_key(ctx, NK_KEY_SCROLL_UP, down);
		}
		else if (sym == SDLK_z)
			nk_input_key(ctx, NK_KEY_TEXT_UNDO, down && state[SDL_SCANCODE_LCTRL]);
		else if (sym == SDLK_r)
			nk_input_key(ctx, NK_KEY_TEXT_REDO, down && state[SDL_SCANCODE_LCTRL]);
		else if (sym == SDLK_c)
			nk_input_key(ctx, NK_KEY_COPY, down && state[SDL_SCANCODE_LCTRL]);
		else if (sym == SDLK_v)
			nk_input_key(ctx, NK_KEY_PASTE, down && state[SDL_SCANCODE_LCTRL]);
		else if (sym == SDLK_x)
			nk_input_key(ctx, NK_KEY_CUT, down && state[SDL_SCANCODE_LCTRL]);
		else if (sym == SDLK_b)
			nk_input_key(ctx, NK_KEY_TEXT_LINE_START, down && state[SDL_SCANCODE_LCTRL]);
		else if (sym == SDLK_e)
			nk_input_key(ctx, NK_KEY_TEXT_LINE_END, down && state[SDL_SCANCODE_LCTRL]);
		else if (sym == SDLK_UP)
			nk_input_key(ctx, NK_KEY_UP, down);
		else if (sym == SDLK_DOWN)
			nk_input_key(ctx, NK_KEY_DOWN, down);
		else if (sym == SDLK_LEFT) {
			if (state[SDL_SCANCODE_LCTRL])
				nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, down);
			else nk_input_key(ctx, NK_KEY_LEFT, down);
		}
		else if (sym == SDLK_RIGHT) {
			if (state[SDL_SCANCODE_LCTRL])
				nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, down);
			else nk_input_key(ctx, NK_KEY_RIGHT, down);
		}
		else return 0;
		return 1;
	}
	else if (evt->type == SDL_MOUSEBUTTONDOWN || evt->type == SDL_MOUSEBUTTONUP) {
		/* mouse button */
		int down = evt->type == SDL_MOUSEBUTTONDOWN;
		const int x = evt->button.x, y = evt->button.y;
		if (evt->button.button == SDL_BUTTON_LEFT) {
			if (evt->button.clicks > 1)
				nk_input_button(ctx, NK_BUTTON_DOUBLE, x, y, down);
			nk_input_button(ctx, NK_BUTTON_LEFT, x, y, down);
		}
		else if (evt->button.button == SDL_BUTTON_MIDDLE)
			nk_input_button(ctx, NK_BUTTON_MIDDLE, x, y, down);
		else if (evt->button.button == SDL_BUTTON_RIGHT)
			nk_input_button(ctx, NK_BUTTON_RIGHT, x, y, down);
		return 1;
	}
	else if (evt->type == SDL_MOUSEMOTION) {
		/* mouse motion */
		if (ctx->input.mouse.grabbed) {
			int x = (int)ctx->input.mouse.prev.x, y = (int)ctx->input.mouse.prev.y;
			nk_input_motion(ctx, x + evt->motion.xrel, y + evt->motion.yrel);
		}
		else nk_input_motion(ctx, evt->motion.x, evt->motion.y);
		return 1;
	}
	else if (evt->type == SDL_TEXTINPUT) {
		/* text input */
		nk_glyph glyph;
		memcpy(glyph, evt->text.text, NK_UTF_SIZE);
		nk_input_glyph(ctx, glyph);
		return 1;
	}
	else if (evt->type == SDL_MOUSEWHEEL) {
		/* mouse wheel */
		nk_input_scroll(ctx, nk_vec2((float)evt->wheel.x, (float)evt->wheel.y));
		return 1;
	}
	return 0;
}
/*
static void
die(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputs("\n", stderr);
	exit(EXIT_FAILURE);
}

static long
timestamp(void)
{
	struct timeval tv;
	if (gettimeofday(&tv, NULL) < 0) return 0;
	return (long)((long)tv.tv_sec * 1000 + (long)tv.tv_usec / 1000);
}

static void
sleep_for(long t)
{
	struct timespec req;
	const time_t sec = (int)(t / 1000);
	const long ms = t - (sec * 1000);
	req.tv_sec = sec;
	req.tv_nsec = ms * 1000000L;
	while (-1 == nanosleep(&req, &req));
}*/


int main(int argc, char* argv[])
{	   
	

	SDL_GLContext glContext;
	int win_width, win_height;
	int running = 1;

	int width = 640;
	int height = 480;


	/* Framebuffer emulator */
	/* SDL setup */
	SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
	SDL_Init(SDL_INIT_VIDEO);	
	win = SDL_CreateWindow("Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);
	renderer = SDL_CreateRenderer(win, -1, 0);
	screen = SDL_CreateRGBSurface(0, width, height, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
	SDL_GetWindowSize(win, &win_width, &win_height);

	struct rawfb_context *rawfb;
	void *fb = NULL;
	unsigned char tex_scratch[512 * 512];
	
	fb = screen->pixels;
		
	
	/* GUI */
	rawfb = nk_rawfb_init(fb, tex_scratch, width, height, width * 4);
	if (!rawfb) running = 0;

#ifdef INCLUDE_STYLE
	/*set_style(ctx, THEME_WHITE);*/
	/*set_style(ctx, THEME_RED);*/
	/*set_style(ctx, THEME_BLUE);*/
	/*set_style(ctx, THEME_DARK);*/
#endif

	while (running) {
		/* Input */
		SDL_Event evt;
		//started = timestamp();
		nk_input_begin(&rawfb->ctx);
		while (SDL_PollEvent(&evt)) {
			if (evt.type == SDL_QUIT) goto cleanup;
			nk_sdl_handle_event(&evt, &rawfb->ctx);
		}
		nk_input_end(&rawfb->ctx);

		/* GUI */
		if (nk_begin(&rawfb->ctx, "Demo", nk_rect(50, 50, 200, 200),
			NK_WINDOW_BORDER | NK_WINDOW_MOVABLE |
			NK_WINDOW_CLOSABLE | NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE)) {
			enum { EASY, HARD };
			static int op = EASY;
			static int property = 20;

			nk_layout_row_static(&rawfb->ctx, 30, 80, 1);
			if (nk_button_label(&rawfb->ctx, "button"))
				printf("button pressed\n");
			nk_layout_row_dynamic(&rawfb->ctx, 30, 2);
			if (nk_option_label(&rawfb->ctx, "easy", op == EASY)) op = EASY;
			if (nk_option_label(&rawfb->ctx, "hard", op == HARD)) op = HARD;
			nk_layout_row_dynamic(&rawfb->ctx, 25, 1);
			nk_property_int(&rawfb->ctx, "Compression:", 0, &property, 100, 10, 1);
		}
		nk_end(&rawfb->ctx);
		if (nk_window_is_closed(&rawfb->ctx, "Demo")) break;

		/* -------------- EXAMPLES ---------------- */
#ifdef INCLUDE_CALCULATOR
		calculator(&rawfb->ctx);
#endif
#ifdef INCLUDE_OVERVIEW
		overview(&rawfb->ctx);
#endif
#ifdef INCLUDE_NODE_EDITOR
		node_editor(&rawfb->ctx);
#endif
		/* ----------------------------------------- */

		/* Draw framebuffer */
		nk_rawfb_render(rawfb, nk_rgb(30, 30, 30), 1);

		/* Emulate framebuffer */
		SDL_UpdateTexture(texture, NULL, screen->pixels, screen->pitch);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);

		/* Timing */
		//dt = timestamp() - started;
		//if (dt < DTIME)
			//sleep_for(DTIME - dt);
	}

cleanup:
	nk_rawfb_shutdown(rawfb);
	SDL_DestroyTexture(texture);
	SDL_FreeSurface(screen);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(win);
	SDL_Quit();

	return 0;
}