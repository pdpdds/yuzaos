#ifndef SGLLIGHT_H
#define SGLLIGHT_H

#include "SGLColor.h"
#include "SGLVector3D.h"
class SGLLight
{
public:
	SGLColor    color;
	SGLVector3D position;
	SGLVector3D ambient;
	SGLVector3D diffuse;
public:
	SGLLight(void)
	{}
	~SGLLight(void)
	{}
};

#endif
