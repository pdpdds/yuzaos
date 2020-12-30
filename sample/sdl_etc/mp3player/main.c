//BEGIN PROGRAM HEAD
//BEGIN DESCRIPTION
/* SDL2_simple_music_player:
 * Plays mp3-music
 * Left mouse button on button / or space switches play and pause.
 * Left mouse button on hold button or
 * backspace stops music (rewind to start).
 *
 */
 //END   DESCRIPTION

 //BEGIN INCLUDES
 //local headers
#include "helper.h"
//END   INCLUDES

//BEGIN CPP DEFINITIONS
//END   CPP DEFINITIONS

//BEGIN DATASTRUCTURES
//END	DATASTRUCTURES

//BEGIN GLOBALS
int ww;
int wh;
Mix_Music* Music = NULL;

//BEGIN VISIBLES
SDL_Surface* temp_surface = NULL;

SDL_Texture* bg = NULL;
SDL_Rect 	bg_dst;

SDL_Texture* slot = NULL;
SDL_Rect 	slot_dst;

SDL_Texture* fillbar = NULL;
SDL_Rect 	fillbar_dst;

SDL_Texture* buttons = NULL;
SDL_Rect 	src_rect_pause;
SDL_Rect 	src_rect_halt;
SDL_Rect 	src_rect_play;

SDL_Rect  	dst_rect_play_pause;
SDL_Rect  	dst_rect_halt;

SDL_Rect 	src_rect;

SDL_Texture* button = NULL;
SDL_Rect 	button_dst;

SDL_Texture* shadow_top = NULL;
SDL_Rect 	shadow_top_dst;

SDL_Texture* shadow_bot = NULL;
SDL_Rect 	shadow_bot_dst;

SDL_Rect 	button_bounds_dst;

TTF_Font* font = NULL;
SDL_Color color = { 255,0,0,255 };
SDL_Texture* text = NULL;
SDL_Rect 	text_dst;
//END 	VISIBLES

int value;
char string[4];

//BEGIN MOUSE
SDL_Point	mouse;
SDL_bool 	mouse_follow = SDL_FALSE;
SDL_Point 	mouse_offset;
//END 	MOUSE

//END   GLOBALS

//BEGIN FUNCTION PROTOTYPES
void assets_in(void);
void assets_out(void);
void render_value(void);
void get_value(void);
void init_rect(SDL_Rect*, int, int, int, int);
SDL_bool PointInCircle(SDL_Point*, SDL_Rect*);
//END	FUNCTION PROTOTYPES

/* DEFINED PROGRESS GOALS
 *
 * todo todo todo
 *
 */
 //END 	PROGRAM HEAD

 //BEGIN MAIN FUNCTION
int main(int argc, char* argv[])
{

	(void)argc;
	(void)argv;

	//BEGIN INIT
	init();
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
	assets_in();

	SDL_SetWindowPosition(Window, 0, 0);
	SDL_SetWindowSize(Window, ww, wh);
	SDL_SetWindowTitle(Window, "Simple Music Player");
	SDL_ShowWindow(Window);

	SDL_Event event;
	int running = 1;
	//END   INIT

	SDL_ShowSimpleMessageBox(0, "SDFSDFFDSSDF", "dsFDSFFSDSDF", Window);

	//BEGIN MAIN LOOP
	while (running) {
		//BEGIN EVENT LOOP
		while (SDL_PollEvent(&event)) {
			SDL_GetMouseState(&mouse.x, &mouse.y);
			if (event.type == SDL_QUIT) {
				running = 0;
			}
			if (event.type == SDL_MOUSEBUTTONDOWN) {
				if (event.button.button == SDL_BUTTON_LEFT && PointInCircle(&mouse, &dst_rect_play_pause)) {
					if (!Mix_PlayingMusic()) {
						Mix_PlayMusic(Music, -1);
						src_rect = src_rect_pause;
					}
					else {
						if (Mix_PausedMusic()) {
							Mix_ResumeMusic();
							src_rect = src_rect_pause;
						}
						else {
							Mix_PauseMusic();
							src_rect = src_rect_play;
						}
					}
				}

				if (event.button.button == SDL_BUTTON_LEFT && PointInCircle(&mouse, &dst_rect_halt)) {
					Mix_HaltMusic();
					src_rect = src_rect_play;
				}

				if (event.button.button == SDL_BUTTON_LEFT && SDL_PointInRect(&mouse, &button_dst) && !mouse_follow) {
					mouse_offset.y = mouse.y - button_dst.y;
					mouse_follow = SDL_TRUE;
				}
			}
			if (mouse_follow && event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
				mouse_follow = SDL_FALSE;
			}
			if (event.type == SDL_KEYDOWN)
			{
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
					running = 0;
					break;

				case SDLK_BACKSPACE:
					Mix_HaltMusic();
					src_rect = src_rect_play;
					break;

				case SDLK_SPACE:
					if (!Mix_PlayingMusic()) {
						Mix_PlayMusic(Music, -1);
						src_rect = src_rect_pause;
					}
					else {
						if (Mix_PausedMusic()) {
							Mix_ResumeMusic();
							src_rect = src_rect_pause;
						}
						else {
							Mix_PauseMusic();
							src_rect = src_rect_play;
						}
					}
					break;

				default:
					break;
				}
			}
		}
		//END   EVENT LOOP
		if (mouse_follow) {
			button_dst.y = mouse.y - mouse_offset.y;

			//BOUNDS CHECK BUTTON
			//TOP
			if (button_dst.y <= button_bounds_dst.y)
				button_dst.y = button_bounds_dst.y;
			//BOT
			if (button_dst.y + button_dst.h >= button_bounds_dst.y + button_bounds_dst.h)
				button_dst.y = (button_bounds_dst.y + button_bounds_dst.h) - button_dst.h;

			//BOUNDS CHECK shadow top
			shadow_top_dst.y = button_dst.y + button_dst.h;
			if (shadow_top_dst.y + shadow_top_dst.h >= slot_dst.y + slot_dst.h)
				shadow_top_dst.y = (slot_dst.y + slot_dst.h) - shadow_top_dst.h;

			fillbar_dst.y = button_dst.y + (button_dst.h / 2);
			fillbar_dst.h = (slot_dst.y + slot_dst.h) - (button_dst.y + button_dst.h / 2);

			get_value();
			render_value();
		}
		//BEGIN RENDERING
		SDL_Delay(66);//~15fps
		SDL_SetRenderDrawColor(Renderer, 111, 111, 111, 255);
		SDL_RenderClear(Renderer);
		SDL_RenderCopy(Renderer, bg, NULL, &bg_dst);
		SDL_RenderCopy(Renderer, slot, NULL, &slot_dst);
		SDL_RenderCopy(Renderer, fillbar, NULL, &fillbar_dst);
		SDL_RenderCopy(Renderer, shadow_top, NULL, &shadow_top_dst);
		SDL_RenderCopy(Renderer, shadow_bot, NULL, &shadow_bot_dst);
		SDL_RenderCopy(Renderer, button, NULL, &button_dst);
		SDL_RenderCopy(Renderer, buttons, &src_rect, &dst_rect_play_pause);
		SDL_RenderCopy(Renderer, buttons, &src_rect_halt, &dst_rect_halt);
		SDL_RenderCopy(Renderer, buttons, &src_rect, &dst_rect_play_pause);
		SDL_RenderCopy(Renderer, buttons, &src_rect_halt, &dst_rect_halt);
		SDL_RenderCopy(Renderer, text, NULL, &text_dst);


		SDL_RenderPresent(Renderer);
		//END   RENDERING

	}
	//END   MAIN LOOP

	assets_out();
	Mix_CloseAudio();
	exit_();
	return 0;

}
//END   MAIN FUNCTION

//BEGIN FUNCTIONS
SDL_bool PointInCircle(SDL_Point* point, SDL_Rect* square)
{
	float x = square->x + square->w / 2;
	float y = square->y + square->h / 2;;
	float r = square->h / 2;

	float dx = x - point->x;
	float dy = y - point->y;

	float distance = sqrtf(dx * dx + dy * dy);

	if (distance < r + 1)
		return SDL_TRUE;
	return SDL_FALSE;
}

void assets_in(void)
{
	Music = Mix_LoadMUS("assets/snd/The.madpix.project_-_Wish_You_Were_Here.mp3");

	//BEGIN BG
	temp_surface = IMG_Load("./assets/gfx/bg.png");
	bg = SDL_CreateTextureFromSurface(Renderer, temp_surface);
	SDL_QueryTexture(bg, NULL, NULL, &bg_dst.w, &bg_dst.h);
	ww = bg_dst.w;
	wh = bg_dst.h;
	bg_dst.x = 0;
	bg_dst.y = 0;
	//END 	BG

	//BEGIN BUTTONS
	temp_surface = IMG_Load("assets/gfx/buttons.png");
	buttons = SDL_CreateTextureFromSurface(Renderer, temp_surface);
	init_rect(&src_rect_pause, 0, 0, 50, 50);
	init_rect(&src_rect_halt, 60, 0, 50, 50);
	init_rect(&src_rect_play, 120, 0, 50, 50);
	src_rect = src_rect_play;
	init_rect(&dst_rect_play_pause, 200, 50, 50, 50);
	init_rect(&dst_rect_halt, 200, 110, 50, 50);
	//END 	BUTTONS

	//BEGIN BUTTON
	temp_surface = IMG_Load("./assets/gfx/button.png");
	button = SDL_CreateTextureFromSurface(Renderer, temp_surface);
	SDL_QueryTexture(button, NULL, NULL, &button_dst.w, &button_dst.h);
	button_dst.x = (120) - (button_dst.w / 2);
	button_dst.y = (wh / 2) - (button_dst.h / 2);
	//END 	BUTTON

	//BEGIN SLOT
	temp_surface = IMG_Load("./assets/gfx/slot.png");
	slot = SDL_CreateTextureFromSurface(Renderer, temp_surface);
	SDL_QueryTexture(slot, NULL, NULL, &slot_dst.w, &slot_dst.h);
	slot_dst.x = (120) - (slot_dst.w / 2);
	slot_dst.y = (wh / 2) - (slot_dst.h / 2);
	//END 	SLOT

	//BEGIN FILLBAR
	temp_surface = IMG_Load("./assets/gfx/fillbar.png");
	fillbar = SDL_CreateTextureFromSurface(Renderer, temp_surface);
	SDL_QueryTexture(fillbar, NULL, NULL, &fillbar_dst.w, &fillbar_dst.h);
	fillbar_dst.x = slot_dst.x + 2;
	fillbar_dst.y = button_dst.y + (button_dst.h / 2);
	fillbar_dst.h = (slot_dst.y + slot_dst.h) - (button_dst.y + button_dst.h / 2);
	//END 	FILLBAR

	//BEGIN SHADOW TOP
	temp_surface = IMG_Load("./assets/gfx/shadow_top.png");
	shadow_top = SDL_CreateTextureFromSurface(Renderer, temp_surface);
	SDL_QueryTexture(shadow_top, NULL, NULL, &shadow_top_dst.w, &shadow_top_dst.h);
	shadow_top_dst.x = slot_dst.x;
	shadow_top_dst.y = button_dst.y + button_dst.h;
	//END 	SHADOW TOP

	//BEGIN SHADOW BOT
	temp_surface = IMG_Load("./assets/gfx/shadow_bot.png");
	shadow_bot = SDL_CreateTextureFromSurface(Renderer, temp_surface);
	SDL_QueryTexture(shadow_bot, NULL, NULL, &shadow_bot_dst.w, &shadow_bot_dst.h);
	shadow_bot_dst.x = slot_dst.x;
	shadow_bot_dst.y = (slot_dst.y + slot_dst.h) - shadow_bot_dst.h;
	//END 	SHADOW BOT

	//BEGIN BOUNDS
	button_bounds_dst.w = button_dst.w;
	button_bounds_dst.h = (button_dst.h * 5) + 10;
	button_bounds_dst.x = button_dst.x;
	button_bounds_dst.y = (button_dst.y - (2 * button_dst.h)) - 4;
	//END 	BOUNDS

	//BEGIN TEXT
	font = TTF_OpenFont("./assets/fonts/NimbusSanL-Regu.ttf", 16);
	get_value();
	render_value();
	//END 	TEXT
}

void assets_out(void)
{
	SDL_DestroyTexture(slot);
	SDL_DestroyTexture(fillbar);
	SDL_DestroyTexture(shadow_top);
	SDL_DestroyTexture(shadow_bot);
	SDL_DestroyTexture(bg);
	SDL_DestroyTexture(button);
	SDL_DestroyTexture(text);
	SDL_DestroyTexture(buttons);
	Mix_FreeMusic(Music);
	SDL_FreeSurface(temp_surface);
}

void render_value(void)
{
	sprintf(string, "%d", value);
	temp_surface = TTF_RenderText_Blended(font, string, color);
	text = SDL_CreateTextureFromSurface(Renderer, temp_surface);
	SDL_QueryTexture(text, NULL, NULL, &text_dst.w, &text_dst.h);
	text_dst.x = (slot_dst.x - text_dst.w / 2) + 7;
	text_dst.y = slot_dst.y + slot_dst.h + 10;

}

void get_value(void)
{

	int min = button_bounds_dst.y + button_dst.h / 2;
	int max = (button_bounds_dst.y + button_bounds_dst.h) - (button_dst.h / 2);
	int b = max - (button_dst.y + button_dst.h / 2);
	float step = ((float)max - (float)min) / 100;
	value = roundf(b / step);
	Mix_VolumeMusic(value);


}

void init_rect(SDL_Rect* rect, int x, int y, int w, int h)
{
	rect->x = x;
	rect->y = y;
	rect->w = w;
	rect->h = h;
}
//END   FUNCTIONS
