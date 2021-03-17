/*#include "sgl.h"
#include <iostream>
using namespace std;

SGLVertex pts[8] = 
{
	SGLVertex(-1.0, -1.0, 1.0, 0.0f, 0.0f), //0
	SGLVertex( 1.0, -1.0, 1.0, 1.0f, 0.0f), //1
	SGLVertex( 1.0,  1.0, 1.0, 1.0f, 1.0f), //2
	SGLVertex(-1.0,  1.0, 1.0, 0.0f, 1.0f), //3
	SGLVertex(-1.0, -1.0, 3.0, 0.0f, 0.0f), //4
	SGLVertex( 1.0, -1.0, 3.0, 1.0f, 0.0f), //5
	SGLVertex( 1.0,  1.0, 3.0, 1.0f, 1.0f), //6
	SGLVertex(-1.0,  1.0, 3.0, 0.0f, 1.0f), //7
};

int texId = -1;
int texId2 = -1;
void Init()
{
	sglViewport(0, 0, 640, 480);
	sglMatrixMode(SGL_PROJECTION);
	sglLoadIdentity();
	sglPerspective(45.0f, 640.0f/480.0f, 10.0f,200.0f);
	sglMatrixMode(SGL_MODELVIEW);
	sglLoadIdentity();
	sglEnable(SGL_ALPHABLEND);
	sglEnable(SGL_ZDEPTHSORT);
	sglEnable(SGL_OBJECT_CULL);
	sglEnable(SGL_BACKFACE_CULL);
	sglEnable(SGL_CLIP_3D);
	sglShadeMode(SGL_FLAT_SHADE);
	texId = sglLoadTextureFromFile("edin.bmp");
	texId2 = sglLoadTextureFromFile("eva.bmp");

	sglEnable(SGL_LIGHT);
	sglLightColor(SGLColor(255, 255, 255));
	sglLight(SGL_AMBIENT, SGLVector3D(0.7, 0.0, 0.7));
	sglLight(SGL_DIFFUSE, SGLVector3D(-1.0, 0.0, 0.0));
}
float a=0;
void Render(unsigned int timeElapsed)
{
	
	sglClearColor(SGL_GRAY);
	sglMatrixMode(SGL_MODELVIEW);
	sglLoadIdentity();

	sglLookAt(0, 0, -20,  0, 0, 0, 0, 1, 0);

	sglPushMatrix();
		sglTranslatef(0.0, 0.0, 0.0);
		sglRotatef(a, 0, 0, 1);
		//sglScalef(3.0f, 3.0f, 3.0f);
		sglBegin(SGL_TRIANGLES);
			sglSetTexture(texId);
			sglTexCoord2f(0.0f, 0.0f);
			sglColor3f(1.0, 0.0, 0.0);
			sglVertex3f(-1.0, -1.0, 1.0); //0
			
			sglTexCoord2f(1.0f, 0.0f);
			sglColor3f(0.0, 1.0, 0.0);
			sglVertex3f( 1.0, -1.0, 1.0); //1
			
			sglTexCoord2f(1.0f, 1.0f);
			sglColor3f(0.0, 0.0, 1.0);
			sglVertex3f( 1.0,  1.0, 1.0); //2

			sglTexCoord2f(0.0f, 0.0f);
			sglColor3f(0.0, 0.0, 1.0);
			sglVertex3f(-1.0, -1.0, 1.0); //3
		
			sglTexCoord2f(1.0f, 1.0f);
			sglVertex3f( 1.0, 1.0,1.0); //4

			sglTexCoord2f(0.0f, 1.0f);
			sglVertex3f(-1.0, 1.0,1.0); //5

			//sglTexCoord2f(1.0f, 1.0f);
			//sglVertex3f( 1.0,  1.0,-1.0); //6

			//sglTexCoord2f(0.0f, 1.0f);
			//sglVertex3f(-1.0,  1.0,-1.0); //7
			
			
		sglEnd();
	sglPopMatrix();

	sglPushMatrix();
		sglTranslatef(0, 0, 0);
		sglScalef(3.0f, 3.0f, 3.0f);
		sglRotatef(a, 1, 0, 1);
		sglBegin(SGL_QUADS);
			sglVertices(pts, 8);
			sglSetTexture(texId);
			sglIndex4i(0, 1, 2, 3); // FRONT
			sglIndex4i(1, 5, 6, 2); // RIGHT
			sglIndex4i(5, 4, 7, 6); // BACK
			sglIndex4i(4, 0, 3, 7); // LEFT
			sglIndex4i(4, 5, 1, 0); // DOWN
			sglIndex4i(3, 2, 6, 7); // UP
		sglEnd();
	sglPopMatrix();

	sglPushMatrix();
		sglTranslatef(10, 0, 0);
		sglRotatef(a, 1, 1, 1);
		sglBegin(SGL_QUADS);
			sglVertices(pts, 8);
			sglSetTexture(texId2);
			sglIndex4i(0, 1, 2, 3); // FRONT
			sglIndex4i(5, 6, 2, 1); // RIGHT
			sglIndex4i(4, 5, 6, 7); // BACK
			sglIndex4i(7, 4, 0, 3); // LEFT
			sglIndex4i(4, 5, 1, 0); // DOWN
			sglIndex4i(6, 7, 3, 2); // UP
		sglEnd();
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
	if(key == '3')
	{
		sglShadeMode(SGL_TEXTURE);
	}
	if(key == '4')
	{
		sglShadeMode(SGL_GOURAUD_SHADE);
	}
	if(key == '5')
	{
		sglEnable(!sglGetState(SGL_ZDEPTHSORT));
	}
	if(key == '6')
	{
		sglEnable(!sglGetState(SGL_OBJECT_CULL));
	}
	if(key == '7')
	{
		sglEnable(!sglGetState(SGL_BACKFACE_CULL));
	}
	if(key == '8')
	{
		sglEnable(!sglGetState(SGL_CLIP_3D));
	}
}

int sglMain(int argc, char *argv[])
{
	sglCreateWindow("TEST", 640, 480);
	sglRegisterRenderFunction(Render);		
	sglRegisterKeyEventFunction(KeyEvent);

	Init();
	return 0;
}*/