#include "SGLDevice.h"
#include "SGLColor.h"
#include "SGLConst.h"
#include <SDL.h>
#include <limits.h> //INT_MAX
#include <math.h>

SGLDevice::~SGLDevice()
{
	if(zbuffer)
		delete [] zbuffer;
}
void SGLDevice::buildSinCosTable(void)
{
	float angle = 0.0f;
	for(int i=0; i<3600; i++)
	{
		SIN[i] = sin(angle*SGL_RAD);
		COS[i] = cos(angle*SGL_RAD);
		angle += 0.1f;
	}
}
extern int sglMain(int argc, char* argv[]);
SGLDevice sglDevice;
#undef main
int main(int argc, char *argv[])
{
	sglMain(argc, argv);
	int x, y;
	SDL_Event event;
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);	
	atexit( SDL_Quit );

	sglDevice.window = SDL_SetVideoMode( sglDevice.windowWidth, sglDevice.windowHeight, sglDevice.windowDepth, SDL_SWSURFACE);
	sglDevice.zbuffer= new int[sglDevice.windowWidth * sglDevice.windowHeight];
	sglClearZBuffer(INT_MAX);
	SDL_WM_SetCaption( sglDevice.windowTitle.c_str() , 0 );
	unsigned int sticks = SDL_GetTicks();
	unsigned int eticks = 0;
	while(1)
	{
		if( SDL_PollEvent(&event) )
		{
			if(event.type == SDL_QUIT)
			{
				break;
			}
			if( event.type == SDL_MOUSEBUTTONDOWN )
			{
				SDL_GetMouseState(&x, &y);
				if(sglDevice.mouseEvent)
				{
					sglDevice.mouseEvent(event.button.which, x, y, true);
				}
			}
			if( event.type == SDL_MOUSEBUTTONUP )
			{
				SDL_GetMouseState(&x, &y);
				if(sglDevice.mouseEvent)
				{
					sglDevice.mouseEvent(event.button.which, x, y, false);
				}
			}
			if( event.type == SDL_KEYDOWN )
			{
				//ESC는 종료키로 설정
				if( event.key.keysym.sym == SDLK_ESCAPE )
				{
					SDL_Event quit;
					quit.type = SDL_QUIT;
					SDL_PushEvent( &quit );
				}
				if(sglDevice.keyboardEvent)
				{
					sglDevice.keyboardEvent(event.key.keysym.sym, true);
				}
			}
			if( event.type == SDL_KEYUP)
			{
				if(sglDevice.keyboardEvent)
				{
					sglDevice.keyboardEvent(event.key.keysym.sym, false);
				}
			}
		}
		if(sglDevice.renderFunction) 
		{
			//첫 실행시 음수 ms 가 넘어가는 것을 막기 위해서 e와 s의 순서를 바꿈.
			//어짜피 흘러간 시간만이 중요하므로 상관 없음
			eticks = SDL_GetTicks(); //end_ticks
			sglDevice.renderFunction(eticks - sticks);
			sticks = SDL_GetTicks(); //start_ticks
		}
		SDL_Flip(sglDevice.window);
	}
	return 0;
}

bool sglCreateWindow(const char* title, int width, int height, int depth)
{
	sglDevice.windowTitle.assign(title);
	sglDevice.windowWidth = width;
	sglDevice.windowHeight = height;
	sglDevice.windowDepth = depth;
	return true;
}
SGLSurface* sglGetWindowSurface(void)
{
	return sglDevice.window;
}
int* sglGetZBuffer(void)
{
	return sglDevice.zbuffer;
}
int  sglGetWindowWidth(void)
{
	return sglDevice.windowWidth;
}
int  sglGetWindowHeight(void)
{
	return sglDevice.windowHeight;
}
int  sglGetColorDepth(void)
{
	return sglDevice.windowDepth;
}
int  sglGetZBufferDepth(void)
{
	return sglDevice.zbufferDepth;
}
void sglRegisterRenderFunction(RenderFuncType render)
{
	sglDevice.renderFunction = render;
}
void sglRegisterKeyEventFunction(KeyEventFuncType keyevent)
{
	sglDevice.keyboardEvent = keyevent;
}
void sglRegisterMouseEventFunction(MouseEventFuncType mouseevent)
{
	sglDevice.mouseEvent = mouseevent;
}
const SGLDevice& sglGetDevice()
{
	return sglDevice;
}
void sglClearSurface(const SGLColor& color)
{
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = sglDevice.window->w;
	rect.h = sglDevice.window->h;
	SDL_FillRect(
		sglDevice.window, 
		&rect,
		SDL_MapRGBA(sglDevice.window->format, color.R, color.G, color.B, color.A)
		);
}
void sglClearZBuffer(const int zvalue)
{
	memset(sglDevice.zbuffer, zvalue, (sglDevice.windowWidth * sglDevice.windowHeight * sglDevice.zbufferDepth));
}