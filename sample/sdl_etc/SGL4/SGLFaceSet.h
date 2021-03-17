#ifndef SGLFACESET_H
#define SGLFACESET_H
/*
 최종 그려질 폴리곤만 담는다.
*/
#include "SGLVertex.h"
#include <vector>
class SGLObject;
class SGLFace
{
public:
	SGLObject* obj;
	int numPoints;
	float depthZ;   //DepthSort 를 위한 Z 값
	SGLVertex v[4]; //한 폴리곤은 4개를 넘지 않는다. 삼각형 또는 사각형만 그리므로
};

class SGLFaceSet
{
public:
	std::vector<SGLFace> faceset;
public:
	SGLFaceSet(void);
	~SGLFaceSet(void);
	void clear(void);
	void addFace(const SGLFace& f);
	SGLFace& operator[](unsigned int index);
	const SGLFace& operator[](unsigned int index) const;
	int size(void) const;
};

#endif