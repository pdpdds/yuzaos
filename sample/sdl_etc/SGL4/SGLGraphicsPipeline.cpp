#include "SGLGraphicsPipeline.h"
#include <math.h>

SGLGraphicsPipeline::SGLGraphicsPipeline(void)
{
	matrixCamera.identity();
	matrixViewport.identity();
	matrixMode = SGL_MODELVIEW;
	viewDistance = 0.0f;
	clipXLeft = 0.0f;
	clipXRight = 0.0f;
	clipYTop = 0.0f;
	clipYBottom = 0.0f;
	clipZNear = 0.0f;
	clipZFar = 0.0f;
	clipImageSpaceXLeft = 0;
	clipImageSpaceXRight = 0;
	clipImageSpaceYTop = 0;
	clipImageSpaceYRight = 0;
	light.color.R = 0;
	light.color.G = 0;
	light.color.B = 0;
}
SGLGraphicsPipeline::~SGLGraphicsPipeline(void)
{
}

/************************/
SGLGraphicsPipeline	gp;
/************************/

void sglLoadIdentity()
{
	gp.matrixStack[gp.matrixMode].setIdentityTopMatrix();
}
void sglMatrixMode(int mode)
{
	gp.matrixMode = ( mode % SGL_MATRIXSTACK_COUNT ); //범위 에러 방지
}
void sglLoadMatrix(SGLMatrix44& m)
{
	m = gp.matrixStack[gp.matrixMode].getTop();
}
void sglMultMatrix(const SGLMatrix44& m)
{
	gp.matrixStack[gp.matrixMode].mul(m);
}
void sglPushMatrix()
{
	gp.matrixStack[gp.matrixMode].push();
}
void sglPopMatrix()
{
	gp.matrixStack[gp.matrixMode].pop();
}
const SGLMatrix44& sglGetModelViewMatrix(void)
{
	return gp.matrixStack[SGL_MODELVIEW].getTop();
}
const SGLMatrix44& sglGetProjectionMatrix(void) 
{
	return gp.matrixStack[SGL_PROJECTION].getTop();
}
const SGLMatrix44& sglGetCameraMatrix(void) 
{
	return gp.matrixCamera;
}
const SGLMatrix44& sglGetViewportMatrix(void) 
{
	return gp.matrixViewport;
}
void sglTranslatef(float x, float y, float z)
{
	gp.matrixStack[gp.matrixMode].getTop().accumTranslate(x, y, z);
}
void sglScalef(float x, float y, float z)
{
	gp.matrixStack[gp.matrixMode].getTop().accumScale(x, y, z);
}
void sglRotatef(float angle, float x, float y, float z)
{
	gp.matrixStack[gp.matrixMode].getTop().accumRotateXYZ(x, y, z, angle);
}
void sglLookAt(float eyex, float eyey, float eyez, float lookatx, float lookaty, float lookatz, float upx, float upy, float upz)
{
	SGLVector3D	eye(eyex, eyey, eyez);
	SGLVector3D	lookat(lookatx, lookaty, lookatz);
	SGLVector3D	up(upx, upy, upz);

	//ZSort 등을 위해 보관
	gp.cameraEye = eye;
	gp.cameraLookAt = lookat;
	gp.cameraUp = up;

	SGLVector3D N;
	SGLVector3D U;
	SGLVector3D V;

	//N = -(lookat - eye);
	N = lookat - eye;
	N.normalize();
	
	//V = up - (up.N)*N;
	//V = V / |V|
	V = up - N*(up.dot(N));
	V.normalize();
	
	U = V.cross(N);
	U.normalize();

	gp.matrixCamera.r1[0] = U.x;
	gp.matrixCamera.r1[1] = U.y;
	gp.matrixCamera.r1[2] = U.z;
	gp.matrixCamera.r1[3] = (-1)*(U.dot(eye));

	gp.matrixCamera.r2[0] = V.x;
	gp.matrixCamera.r2[1] = V.y;
	gp.matrixCamera.r2[2] = V.z;
	gp.matrixCamera.r2[3] = (-1)*(V.dot(eye));

	gp.matrixCamera.r3[0] = N.x;
	gp.matrixCamera.r3[1] = N.y;
	gp.matrixCamera.r3[2] = N.z;
	gp.matrixCamera.r3[3] = (-1)*(N.dot(eye));

	gp.matrixCamera.r4[0] = 0.0f;
	gp.matrixCamera.r4[1] = 0.0f;
	gp.matrixCamera.r4[2] = 0.0f;
	gp.matrixCamera.r4[3] = 1.0f;
}
void sglPerspective(float fovy, float aspect, float znear, float zfar)
{
	float ll, rr, bb, tt;

	tt = znear*tan(fovy*(3.141592f/180.0f));
	bb = -tt;
	ll = bb*aspect;
	rr = tt*aspect;

	sglFrustum(ll, rr, bb, tt, znear, zfar);
}
void sglFrustum(float left, float right, float bottom, float top, float znear, float zfar)
{
	/*if(znear < 0)
		znear *= -1.0f;
	if(zfar < 0)
		zfar *= -1.0f;*/

	//클립핑 범위 설정
	gp.clipXLeft = left;
	gp.clipXRight = right;
	gp.clipYTop = top;
	gp.clipYBottom = bottom;
	gp.clipZNear = znear;
	gp.clipZFar = zfar;
	gp.viewDistance = znear;

	SGLMatrix44 m;
	m.identity();
	m.m11 =  (2*znear)/(right-left);
	m.m13 =  (right+left)/(right-left);
	m.m22 =  (2*znear)/(top-bottom);
	m.m23 =  (top+bottom)/(top-bottom);
	m.m33 = -((zfar+znear)/(zfar-znear));
	m.m34 = -(2*(znear*zfar))/(zfar-znear);
	m.m43 = -1.0;
	gp.matrixStack[SGL_PROJECTION].mul(m);
}
void sglOrtho(float left, float right, float bottom, float top, float znear, float zfar)
{
	if(znear < 0)
		znear *= -1.0f;
	if(zfar < 0)
		zfar *= -1.0f;

	SGLMatrix44 m;
	m.identity();
	m.m11 = 2/(right-left);
	m.m14 = -(right+left)/(right-left);
	m.m22 = 2/(top-bottom);
	m.m24 = -(top+bottom)/(top-bottom);
	m.m33 = -2/(zfar-znear);
	m.m34 = -(zfar+znear)/(zfar-znear);
	gp.matrixStack[SGL_PROJECTION].mul(m);
}
void sglViewport(int x, int y, int width, int height)
{
	gp.clipImageSpaceXLeft = x;
	gp.clipImageSpaceXRight = y;
	gp.clipImageSpaceYTop = width;
	gp.clipImageSpaceYRight = height;

	gp.matrixViewport.identity();
	
	int	  sx  = width - x; //너비
	int	  sy  = height - y;//높이
	int   ox =  x + (sx>>1); //원점x
	int   oy =  y + (sx>>1); //원점y
	
	gp.matrixViewport.m11 = (float)-ox;
	gp.matrixViewport.m14 = (float)(sx>>1);
	gp.matrixViewport.m22 = (float)oy;
	gp.matrixViewport.m24 = (float)(sy>>1);
}

int  sglLoadTextureFromFile(const char* bitmapFilename)
{
	return gp.textures.addTexture(bitmapFilename);
}

void sglLightColor(const SGLColor& c)
{
	gp.light.color = c;
}
void sglLight(int light, const SGLVector3D& intensity)
{
	if(light == SGL_AMBIENT)
	{
		gp.light.ambient = intensity;
	}
	else
	if(light == SGL_DIFFUSE)
	{
		gp.light.diffuse = intensity;
	}
}
void sglLightPos(const SGLVector3D& pos)
{
	gp.light.position = pos;
}