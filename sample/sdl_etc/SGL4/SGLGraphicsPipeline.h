#ifndef SGLGRAPHICSPIPELINE_H
#define SGLGRAPHICSPIPELINE_H

#include "SGLPoint2D.h"
#include "SGLPoint3D.h"
#include "SGLVector3D.h"
#include "SGLMatrix44.h"
#include "SGLMatrixStack.h"
#include "SGLConst.h"
#include "SGLTextureList.h"
#include "SGLLight.h"

void sglLoadIdentity(); //sglLoadIdentity()
void sglMatrixMode(int mode); //sglMatrixMode()
void sglLoadMatrix(SGLMatrix44& m);
void sglMultMatrix(const SGLMatrix44& m);
void sglPushMatrix();
void sglPopMatrix();
const SGLMatrix44& sglGetModelViewMatrix(void);
const SGLMatrix44& sglGetProjectionMatrix(void);
const SGLMatrix44& sglGetCameraMatrix(void);
const SGLMatrix44& sglGetViewportMatrix(void);
void sglTranslatef(float x, float y, float z);
void sglScalef(float x, float y, float z);
void sglRotatef(float angle, float x, float y, float z);
void sglLookAt(float eyex, float eyey, float eyez, float lookatx, float lookaty, float lookatz, float upx, float upy, float upz);
void sglPerspective(float fovy, float aspect, float znear, float zfar);
void sglFrustum(float left, float right, float bottom, float top, float znear, float zfar);
void sglOrtho(float left, float right, float bottom, float top, float znear, float zfar);
void sglViewport(int x, int y, int width, int height);
int  sglLoadTextureFromFile(const char* bitmapFilename);
void sglLightColor(const SGLColor& color);
void sglLight(int light, const SGLVector3D& intensity);
void sglLightPos(const SGLVector3D& pos);

class SGLGraphicsPipeline
{
private:
	SGLMatrixStack	matrixStack[SGL_MATRIXSTACK_COUNT];
	SGLMatrix44		matrixCamera;
	SGLMatrix44		matrixViewport;	
	int				matrixMode;
public:
	float			viewDistance;
	float			clipXLeft;				//절두체 클리핑
	float			clipXRight;				//절두체 클리핑
	float			clipYTop;				//절두체 클리핑
	float			clipYBottom;			//절두체 클리핑
	float			clipZNear;				//절두체 클리핑
	float			clipZFar;				//절두체 클리핑
	int				clipImageSpaceXLeft;	//뷰포트 클리핑
	int				clipImageSpaceXRight;	//뷰포트 클리핑
	int				clipImageSpaceYTop;		//뷰포트 클리핑
	int				clipImageSpaceYRight;	//뷰포트 클리핑
	SGLVector3D		cameraEye;				//뷰포트 클리핑
	SGLVector3D		cameraLookAt;			//뷰포트 클리핑
	SGLVector3D		cameraUp;				//뷰포트 클리핑
	SGLTextureList  textures;
	SGLLight		light;
public:
	SGLGraphicsPipeline(void);
	~SGLGraphicsPipeline(void);

	friend void sglLoadIdentity(); 
	friend void sglMatrixMode(int mode); 
	friend void sglLoadMatrix(SGLMatrix44& m);
	friend void sglMultMatrix(const SGLMatrix44& m);
	friend void sglPushMatrix();
	friend void sglPopMatrix();
	friend const SGLMatrix44& sglGetModelViewMatrix(void);
	friend const SGLMatrix44& sglGetProjectionMatrix(void);
	friend const SGLMatrix44& sglGetCameraMatrix(void);
	friend const SGLMatrix44& sglGetViewportMatrix(void);
	friend void sglTranslatef(float x, float y, float z);
	friend void sglScalef(float x, float y, float z);
	friend void sglRotatef(float angle, float x, float y, float z);
	friend void sglLookAt(float eyex, float eyey, float eyez, float lookatx, float lookaty, float lookatz, float upx, float upy, float upz);
	friend void sglPerspective(float fovy, float aspect, float znear, float zfar);
	friend void sglFrustum(float left, float right, float bottom, float top, float znear, float zfar);
	friend void sglOrtho(float left, float right, float bottom, float top, float znear, float zfar);
	friend void sglViewport(int x, int y, int width, int height);
};
extern SGLGraphicsPipeline	gp;
#endif