#include "graphics.h"


void init_graphics() {

    render_changed = false;

    //SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

	if (SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &render) < 0)
	{
		printf("SDL_CreateWindowAndRenderer Error\n");
		return;
	}

    SDL_SetRenderDrawBlendMode(render, SDL_BLENDMODE_BLEND);

    // texture for render context
    display = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, WINDOW_WIDTH, WINDOW_HEIGHT);

    SDL_SetRenderTarget(render, display);

    // Load font
    gFont = TTF_OpenFont("Inconsolata-Regular.ttf", 30);

	if (gFont == NULL)
	{
		fprintf(stderr, "\nTTF_OpenFont Error:  %s\n", SDL_GetError());
		exit(1);
	}

    //TTF_SetFontHinting(gFont, TTF_HINTING_MONO);
}

void setRenderChanged() {
    render_changed = true;
}

void preRender() {


    SDL_SetRenderTarget(render, display);


}

void updateRender() {

    // lazily update the screen only if render operations are queued
    if(render_changed) {

        SDL_SetRenderTarget(render, NULL);
        SDL_RenderCopy(render, display, NULL, NULL);

        SDL_RenderPresent(render);
        render_changed = false;

    }
}

void draw_block(uint8_t x, uint8_t y, uint32_t color) {

    assert(x >= 0 && x < PLAYFIELD_WIDTH);
    assert(y >= 0 && y < PLAYFIELD_HEIGHT);

    // top-left coords of block
    uint16_t x_tl = x * (BLOCK_SIZE + 1) + 1;
    uint16_t y_tl = y * (BLOCK_SIZE + 1) + 1;

    // top-right coords of block
    uint16_t x_tr = x_tl + BLOCK_SIZE;
    uint16_t y_tr = y_tl;

    // bottom-right coords of block
    uint16_t x_br = x_tl + BLOCK_SIZE;
    uint16_t y_br = y_tl + BLOCK_SIZE;

    // bottom-left coords of block
    uint16_t x_bl = x_tl;
    uint16_t y_bl = y_tl + BLOCK_SIZE;

    boxColor(render, x_tl, y_tl, x_br, y_br, color);

    
    setRenderChanged();

}

void cleanup_graphics() {
    SDL_DestroyRenderer(render);
    SDL_DestroyWindow(window);
}
