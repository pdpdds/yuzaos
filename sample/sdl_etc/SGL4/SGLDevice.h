#ifndef SGLDEVICE_H
#define SGLDEVICE_H

#include <string>
#include <SDL.h>
typedef void (*RenderFuncType)(unsigned int timeElapsed);
typedef void (*KeyEventFuncType)(int key, bool pressed);
typedef void (*MouseEventFuncType)(int button, int x, int y, bool pressed);
typedef SDL_Surface SGLSurface;
class SGLColor;
class SGLDevice
{
public:
	SGLSurface* window;
	int *zbuffer;
	unsigned int windowWidth;
	unsigned int windowHeight;
	unsigned int windowDepth;
	unsigned int zbufferDepth;
	std::string  windowTitle;
	MouseEventFuncType mouseEvent;
	KeyEventFuncType keyboardEvent;
	RenderFuncType renderFunction;
	//SIN, COS TABLE, 0.1µµ ¥‹¿ß
	float SIN[3600];
	float COS[3600];
private:
	void buildSinCosTable(void);
public:
	SGLDevice(void) : window(0), windowWidth(0), windowHeight(0), windowDepth(0), windowTitle(""), 
		mouseEvent(0), keyboardEvent(0), renderFunction(0), zbuffer(0)
	{
		buildSinCosTable();
	}
	~SGLDevice(void);
};
extern SGLDevice sglDevice;
bool sglCreateWindow(const char* title, int width, int height, int depth=32);
SGLSurface* sglGetWindowSurface(void);
int* sglGetZBuffer(void);
int  sglGetWindowWidth(void);
int  sglGetWindowHeight(void);
int  sglGetColorDepth(void);
int  sglGetZBufferDepth(void);
void sglRegisterRenderFunction(RenderFuncType render);
void sglRegisterKeyEventFunction(KeyEventFuncType keyevent);
void sglRegisterMouseEventFunction(MouseEventFuncType mouseevent);
void sglClearSurface(const SGLColor& color);
void sglClearZBuffer(const int zvalue);
const SGLDevice& sglGetDevice();

#endif