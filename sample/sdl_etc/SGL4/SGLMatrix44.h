#ifndef SGLMATRIX44_H
#define SGLMATRIX44_H

#include "SGLVector3D.h"
#include "SGLPoint3D.h"
class SGLMatrix44
{
public:
	union
	{
		struct
		{
			float m11, m12, m13, m14;
			float m21, m22, m23, m24;
			float m31, m32, m33, m34;
			float m41, m42, m43, m44;
		};
		struct
		{
			float r1[4];
			float r2[4];
			float r3[4];
			float r4[4];
		};
		struct
		{
			float m[4][4];
		};
	};
public:
	SGLMatrix44(void);
	SGLMatrix44(const SGLMatrix44& matrix);
	~SGLMatrix44(void);

	void identity(void);
	void transpose(void);
	void rotateX(float angle);
	void rotateY(float angle);
	void rotateZ(float angle);
	void rotateXYZ(float x, float y, float z, float angle);
	void translate(float x, float y, float z);
	void scale(float x, float y, float z);
	
	//accum
	void accumRotateX(float angle);
	void accumRotateY(float angle);
	void accumRotateZ(float angle);
	void accumRotateXYZ(float x, float y, float z, float angle);
	void accumTranslate(float x, float y, float z);
	void accumScale(float x, float y, float z);

	//MATRIX44 = MATRIX44 + MATRIX44
	SGLMatrix44 operator+(const SGLMatrix44& matrix) const;
	//const SGLMatrix44& operator+(const SGLMatrix44& matrix);
	void operator+=(const SGLMatrix44& matrix);

	//MATRIX44 = MATRIX44 - MATRIX44
	SGLMatrix44 operator-(const SGLMatrix44& matrix) const;
	//const SGLMatrix44& operator-(const SGLMatrix44& matrix);
	void operator-=(const SGLMatrix44& matrix);

	//MATRIX44 = MATRIX44 * MATRIX44
	SGLMatrix44 operator*(const SGLMatrix44& matrix) const;
	//const SGLMatrix44& operator*(const SGLMatrix44& matrix);
	void operator*=(const SGLMatrix44& matrix);

	//VECTOR3D = MATRIX44 * VECTOR3D
	SGLVector3D operator*(const SGLVector3D& v) const;

	//POINT3D = MATRIX44 * POINT3D
	SGLPoint3D operator*(const SGLPoint3D& pt) const;
	
};


#endif