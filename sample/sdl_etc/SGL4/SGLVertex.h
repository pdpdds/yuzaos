#ifndef SGLVERTEX_H
#define SGLVERTEX_H

#include "SGLPoint3D.h"
#include "SGLVector3D.h"
#include "SGLColor.h"
class SGLVertex
{
public:
	SGLPoint3D pos;		//위치
	float u, v;			//텍스춰좌표
	SGLVector3D normal; //고러드쉐이딩할 때, 정점의 법선
	SGLColor color;		//정점의 색상
public:
	SGLVertex(void);
	SGLVertex(float x, float y, float z, const SGLColor& c = SGLColor(0, 0, 0), SGLVector3D& nv = SGLVector3D(), float u=0.0f, float v=0.0f);
	SGLVertex(float x, float y, float z, float u, float v);
	SGLVertex(const SGLPoint3D& pt, const SGLColor& c = SGLColor(0, 0, 0), SGLVector3D& nv = SGLVector3D(), float u=0.0f, float v=0.0f);
	SGLVertex(const SGLVertex& v);
	~SGLVertex(void);

	void setTexCoord(float u, float v);
	void setColor(const SGLColor& c);
	void setNormal(const SGLVector3D& nv);
};

#endif
