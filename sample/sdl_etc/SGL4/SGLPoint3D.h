#ifndef SGLPOINT3D_H
#define SGLPOINT3D_H
#include "SGLVector3D.h"

class SGLPoint3D
{
public:
	float x, y, z, w;
public:
	SGLPoint3D(void) : x(0.0f), y(0.0f), z(0.0f), w(1.0f)
	{}
	SGLPoint3D(const SGLPoint3D& pt) : x(pt.x), y(pt.y), z(pt.z), w(1.0f)
	{}
	SGLPoint3D(float X, float Y, float Z) : x(X), y(Y), z(Z), w(1.0f)
	{}
	~SGLPoint3D(void)
	{}

	float getLength(const SGLPoint3D& pt) const;
	float getLength(void) const;

	//VECTOR3D = PT3D - PT3D
	SGLVector3D operator-(const SGLPoint3D& pt) const;
	
	//pt + pt
	SGLPoint3D operator+(const SGLPoint3D& pt) const;
	//const SGLPoint3D& operator+(const SGLPoint3D& pt);
	void operator+=(const SGLPoint3D& pt);

	//pt + float
	SGLPoint3D operator+(const float f) const;
	//const SGLPoint3D& operator+(const float f);
	void operator+=(const float f);

	

	//pt - float
	SGLPoint3D operator-(const float f) const;
	//const SGLPoint3D& operator-(const float f);
	void operator-=(const float f);
	
	//pt * f
	SGLPoint3D operator*(const float f) const;
	//const SGLPoint3D& operator*(const float f);
	void operator*=(const float f);
};

#endif
