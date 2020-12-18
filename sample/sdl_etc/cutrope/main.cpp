/*
 * main.cpp
 *
 *  Created on: Oct 26, 2016
 *      Author: myths
 */
#include"Environment.h"

#include "Scene1.h"
#include "Scene2.h"
#include "Scene3.h"

SDL_Window *window;
SDL_Renderer *render;
SDL_Texture *textureBg, *textureBtn1, *textureBtn2, *textureBtn3;
SDL_Rect rectBg, rectBtn1, rectBtn2, rectBtn3;

int getPos(int x, int y) {
	if (x >= rectBtn1.x && x <= rectBtn1.x + rectBtn1.w && y >= rectBtn1.y
			&& y <= rectBtn1.y + rectBtn1.h) {
		return 1;
	} else if (x >= rectBtn2.x && x <= rectBtn2.x + rectBtn2.w
			&& y >= rectBtn2.y && y <= rectBtn2.y + rectBtn2.h) {
		return 2;
	} else if (x >= rectBtn3.x && x <= rectBtn3.x + rectBtn3.w
			&& y >= rectBtn3.y && y <= rectBtn3.y + rectBtn3.h) {
		return 3;
	}
	return 0;
}
void paint() {
	SDL_RenderClear(render);
	SDL_RenderCopy(render, textureBg, NULL, &rectBg);
	SDL_RenderCopy(render, textureBtn1, NULL, &rectBtn1);
	SDL_RenderCopy(render, textureBtn2, NULL, &rectBtn2);
	SDL_RenderCopy(render, textureBtn3, NULL, &rectBtn3);
	SDL_RenderPresent(render);
}
int main() {

	SDL_Init(SDL_INIT_EVERYTHING);
	window = SDL_CreateWindow("Cut the rope demo", SDL_WINDOWPOS_UNDEFINED,
	SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
	if (window == NULL) {
		printf("Could not create window: %s\n", SDL_GetError());
		return 1;
	}
	render = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
	if (render == NULL) {
		printf("Could not create render: %s\n", SDL_GetError());
		return 1;
	}
	SDL_Surface *surface = SDL_GetWindowSurface(window);
	rectBg = {0, 0, WIDTH, HEIGHT};
	SDL_FillRect(surface, &rectBg, SDL_MapRGB(surface->format, 111, 111, 111));
	textureBg = SDL_CreateTextureFromSurface(render, surface);
	SDL_FillRect(surface, &rectBg, SDL_MapRGB(surface->format, 255, 255, 255));
	textureBtn1 = SDL_CreateTextureFromSurface(render, surface);
	SDL_FillRect(surface, &rectBg, SDL_MapRGB(surface->format, 0, 0, 255));
	textureBtn2 = SDL_CreateTextureFromSurface(render, surface);
	SDL_FillRect(surface, &rectBg, SDL_MapRGB(surface->format, 255, 0, 0));
	textureBtn3 = SDL_CreateTextureFromSurface(render, surface);
	SDL_FreeSurface(surface);
	rectBtn1= {WIDTH/5*2,HEIGHT/7,WIDTH/5,HEIGHT/7};
	rectBtn2= {WIDTH/5*2,HEIGHT/7*3,WIDTH/5,HEIGHT/7};
	rectBtn3= {WIDTH/5*2,HEIGHT/7*5,WIDTH/5,HEIGHT/7};

	bool quit = false;
	int signal = -1;
	while (!quit) {
		paint();
		SDL_Event e;
		if (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT: {
				quit = true;
				break;
			}
			case SDL_MOUSEBUTTONDOWN: {
				int btn = getPos(e.button.x, e.button.y);
				if (btn == 1) {
					Scene1 sceneOne(window, render);
					signal = sceneOne.execute();
				} else if (btn == 2) {
					Scene2 sceneTwo(window, render);
					signal = sceneTwo.execute();
				} else if (btn == 3) {
					Scene3 sceneThree(window, render);
					signal = sceneThree.execute();
				}
			}
			default: {
				break;
			}

			}
		}
		switch (signal) {
		case -1:
			break;
		case 0:
			quit = true;
			break;
		case 1:
			break;
		}
	}

	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(render);
	SDL_DestroyTexture(textureBg);
	SDL_DestroyTexture(textureBtn1);
	SDL_DestroyTexture(textureBtn2);
	SDL_DestroyTexture(textureBtn3);
	SDL_Quit();
}
