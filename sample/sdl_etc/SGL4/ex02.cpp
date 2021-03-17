
#include "sgl.h"
#include <iostream>
using namespace std;


int texId = -1;
int texId2 = -1;
SGLObjModel pot;
SGLMD2Model md2;
bool renderingObj = true;
void Init()
{
	sglViewport(0, 0, 800, 600);
	sglMatrixMode(SGL_PROJECTION);
	sglLoadIdentity();
	sglPerspective(45.0f, 800.0f/600.0f, 10.0f,300.0f);
	sglMatrixMode(SGL_MODELVIEW);
	sglLoadIdentity();
	sglEnable(SGL_ALPHABLEND);
	sglEnable(SGL_ZDEPTHSORT);
	sglEnable(SGL_OBJECT_CULL);
	sglEnable(SGL_BACKFACE_CULL);
	sglEnable(SGL_CLIP_3D);
	sglShadeMode(SGL_FRAME);
	texId = sglLoadTextureFromFile("edin.bmp");
	texId2 = sglLoadTextureFromFile("eva.bmp");

	pot.Load("gourd.obj");
	md2.Load("hellpig.md2");
	md2.SetSkin(sglLoadTextureFromFile("hellpig.bmp"));

	sglEnable(SGL_LIGHT);
	sglLightColor(SGLColor(255, 255, 255));
	sglLight(SGL_AMBIENT, SGLVector3D(0.7, 0.0, 0.7));
	sglLight(SGL_DIFFUSE, SGLVector3D(-1.0, 0.0, 0.0));
}
float a=0;
unsigned int s=0;
void Render(unsigned int timeElapsed)
{
	
	sglClearColor(SGL_GRAY);
	sglMatrixMode(SGL_MODELVIEW);
	sglLoadIdentity();

	sglLookAt(0, 0, -20,  0, 0, 100, 0, 1, 0);

	sglPushMatrix();
		if(renderingObj)
		{
			sglTranslatef(0.0, 0.0, 0.0);
			sglScalef(2, 2, 2);
			sglRotatef(a, 0, 1, 0);
			pot.Render();
		}
		else
		{
			sglTranslatef(0.0, 0.0, 30.0);
			sglScalef(6, 6, 6);
			sglRotatef(a, 0, 1, 0);
			md2.Animate(100, s); s++;
		}
	sglPopMatrix();


	sglSwapBuffer();
	a += 3;
	if( a > 360) a = 0.0f;
}

void KeyEvent(int key, bool pressed)
{
	if(key == '1')
	{
		sglShadeMode(SGL_FRAME);
	}
	if(key == '2')
	{
		sglShadeMode(SGL_FLAT_SHADE);
	}
	if(key == '3' && !renderingObj)
	{
		sglShadeMode(SGL_TEXTURE);
	}
	if(key == '4')
	{
		sglShadeMode(SGL_GOURAUD_SHADE);
	}
	if(key == SDLK_F1)
	{
		sglShadeMode(SGL_FRAME);
		renderingObj = true;
	}
	if(key == SDLK_F2)
	{
		renderingObj = false;
	}
	
}

int sglMain(int argc, char *argv[])
{
	sglCreateWindow("SGL : Rendering Model File", 800, 600);
	sglRegisterRenderFunction(Render);		
	sglRegisterKeyEventFunction(KeyEvent);
	Init();
	return 0;
}
