#ifndef SGLOBJECT_H
#define SGLOBJECT_H

#include "SGLVertexList.h"
#include "SGLPolygonList.h"


//객체정의
void sglBegin(int objectType);
void sglSetTexture(int textureId);
void sglVertices(const SGLVertex* vlist, int count);
void sglVertex3f(float x, float y, float z);
void sglIndexes(const int* vilist, int count);
void sglIndex3i(int v1, int v2, int v3);
void sglIndex4i(int v1, int v2, int v3, int v4);
void sglColor3f(float r, float g, float b); //a = 1.0f
void sglColor4f(float r, float g, float b, float a);
void sglNormal3f(float x, float y, float z);
void sglTexCoord2f(float u, float v);
void sglSwapBuffer(void);
void sglEnd();

class SGLObject
{
public:
	int   objectType; //오브젝트 타입
	bool  visible; //보이는 오브젝트인가?
	float radius;  //절두체 클리핑을 위한 변수, 클리핑을 통해서 visible 이 설정된다.
	int numPolygons;
	int numVertices;
	int numTextures;
	
	//인덱스정의용
	bool indexedType;
	SGLPolygonList	plist;
	SGLVertexList	localVertices;
	SGLVertexList	worldVertices;
	SGLVertexList	cameraVertices;

	//정점정의용
	SGLVertexPolygonList vplist;


	SGLTextureIdList textureIds;
	SGLPoint3D		worldPos; //월드좌표
public:
	SGLObject(void) : objectType(0), numTextures(0), visible(true), radius(0.0f), numPolygons(0), numVertices(0), indexedType(false)
	{}
	SGLObject(int type) : objectType(type), numTextures(0), visible(true), radius(0.0f), numPolygons(0), numVertices(0), indexedType(false)
	{}
	~SGLObject(void)
	{}
};



#endif
