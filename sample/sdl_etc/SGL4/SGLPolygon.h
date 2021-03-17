#ifndef SGLPOLYGON_H
#define SGLPOLYGON_H
#include "SGLVertexList.h"
#include "SGLVertexIndexList.h"
#include "SGLVector3D.h"
#include "SGLVertex.h"
#include "SGLColor.h"
//인덱스용 폴리곤
class SGLPolygon
{
public:
	bool visible;	//보이는 면인가?
	bool clipped;	//클리핑 되었는가?
	int numVertex;
	SGLColor intencity;
	SGLVertexIndexList indexList;	
	//SGL
	SGLVector3D normal; //법선벡터
	//int textureId; //텍스춰리스트의 인덱스
public:
	SGLPolygon(void) : visible(true), clipped(false), numVertex(0)
	{}
	~SGLPolygon(void)
	{}
	SGLPolygon(int v1, int v2, int v3);
	SGLPolygon(int v1, int v2, int v3, int v4);
};

//정점용 폴리곤
class SGLVertexPolygon
{
public:
	bool visible;	//보이는 면인가?
	bool clipped;	//클리핑 되었는가?
	int numVertex;
	SGLColor intencity;
	SGLVertexList	localVertices;
	SGLVertexList	worldVertices;
	SGLVertexList	cameraVertices;
	//SGL
	SGLVector3D normal; //법선벡터
	//int textureId; //텍스춰리스트의 인덱스
public:
	SGLVertexPolygon(void) : visible(true), clipped(false), numVertex(0)
	{}
	~SGLVertexPolygon(void)
	{}
	SGLVertexPolygon(const SGLVertex& v1, const SGLVertex& v2, const SGLVertex& v3);
	SGLVertexPolygon(const SGLVertex& v1, const SGLVertex& v2, const SGLVertex& v3, const SGLVertex& v4);
};
#endif