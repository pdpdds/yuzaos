#include <stdio.h>
#include <memory.h>
#include "GL/gl.h"
#include "GL/glu.h"
#include <stdlib.h>
#include "SDL2/SDL.h"
#include "zbuffer.h"
#include <sprintf.h>
#include <systemcall_impl.h>

#define TICK_INTERVAL    30

static Uint32 next_time;

Uint32 time_left(void)
{
	Uint32 now;

	now = SDL_GetTicks();
	if (next_time <= now)
		return 0;
	else
		return next_time - now;
}

int main(int argc, char** argv)
{
	printf("%tinygl test!!\n", (char*)argv[0]);

	// initialize SDL video:
	int winSizeX = 640;
	int winSizeY = 480;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("ERROR: cannot initialize SDL video.\n");
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow("TinyGL test - 'Gears'",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		winSizeX, winSizeY,
		SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_SOFTWARE);

	SDL_Surface* screen = SDL_CreateRGBSurface(0, winSizeX, winSizeY, 16, 0, 0, 0, 0);
	if (screen == NULL)
	{
		SDL_Log("SDL_CreateRGBSurface() failed: %s", SDL_GetError());
		exit(1);
	}

	SDL_Texture* screenTexture;

	// initialize TinyGL:
	unsigned int pitch;
	int	mode;
	switch (screen->format->BitsPerPixel)
	{
	case  8:
		printf("ERROR: Palettes are currently not supported.\n");
		return 1;
	case 16:
		pitch = screen->pitch;
		mode = ZB_MODE_5R6G5B;
		screenTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, winSizeX, winSizeY);
		break;
	case 24:
		pitch = (screen->pitch * 2) / 3;
		mode = ZB_MODE_RGB24;
		/*!!! bug with 24-bit color :-( !!!*/
		screenTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, winSizeX, winSizeY);
		break;
	case 32:
		pitch = screen->pitch / 2;
		mode = ZB_MODE_RGBA;
		screenTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, winSizeX, winSizeY);
		break;
	default:
		return 1;
		break;
	}
	ZBuffer* frameBuffer = ZB_open(winSizeX, winSizeY, mode, 0, 0, 0, 0);
	glInit(frameBuffer);

	// set viewport
	glViewport(0, 0, winSizeX, winSizeY);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// main loop:
	int frames = 0;
	int x = 0;
	double  t, t0, fps;
	char    titlestr[200];
	int running = GL_TRUE;
	t0 = (double)SDL_GetTicks();
	next_time = SDL_GetTicks() + TICK_INTERVAL;

	while (running) {
		// calculate and display FPS (frames per second):
		t = (double)SDL_GetTicks();
		if ((t - t0) > 100.0 || frames == 0) {
			fps = (double)frames / (t - t0);
			sprintf(titlestr, "Spinning Triangle (%.1f FPS)", fps);
			//SDL_WM_SetCaption(titlestr,0);
			SDL_SetWindowTitle(window, titlestr);
			t0 = t;
			frames = 0;
		}
		++frames;

		// Clear color buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Select and setup the projection matrix
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(65.0f, (GLfloat)winSizeX / (GLfloat)winSizeY, 1.0f, 100.0f);

		// Select and setup the modelview matrix
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotatef(-90, 1, 0, 0);
		glTranslatef(0, 0, -1.0f);

		// Draw a rotating colorful triangle
		glTranslatef(0.0f, 14.0f, 0.0f);
		glRotatef(0.3 * (GLfloat)x + (GLfloat)t/10.0f, 0.0f, 0.0f, 1.0f);
		glBegin(GL_TRIANGLES);
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3f(-5.0f, 0.0f, -4.0f);
		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex3f(5.0f, 0.0f, -4.0f);
		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex3f(0.0f, 0.0f, 6.0f);
		glEnd();

		// swap buffers:
		ZB_copyFrameBuffer(frameBuffer, screen->pixels, pitch);
		SDL_UpdateTexture(screenTexture, NULL, screen->pixels, screen->pitch);

		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, screenTexture, NULL, NULL);
		SDL_RenderPresent(renderer);
		//SDL_Delay(time_left());
		next_time += TICK_INTERVAL;

		// check if the ESC key was pressed or the window was closed:
		SDL_Event evt;
		while (SDL_PollEvent(&evt)) switch (evt.type) {
		case SDL_KEYDOWN:
			if (evt.key.keysym.sym == SDLK_ESCAPE)
				running = 0;
			break;
		case SDL_QUIT:
			running = 0;
			break;
		}
	}
	// cleanup:
	ZB_close(frameBuffer);
	if (SDL_WasInit(SDL_INIT_VIDEO))
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
	SDL_Quit();
	
	printf("comlete!!\n");

	return 0;
}