#include "SGLVector3D.h"
#include <math.h>
float SGLVector3D::length(void) const
{
	return sqrt( float( x*x + y*y + z*z ) );
}
void  SGLVector3D::normalize(void)
{
	float len = length();
	x /= len;
	y /= len;
	z /= len;
}
float SGLVector3D::dot(const SGLVector3D& v) const
{
	return ( x*v.x + y*v.y + z*v.z);
}
SGLVector3D SGLVector3D::cross(const SGLVector3D& v) const
{
	return SGLVector3D(
		  y*v.z - z*v.y,
		  z*v.x - x*v.z,
		  x*v.y - y*v.x
		);
}

//VECTOR3D = VECTOR3D - VECTOR3D
SGLVector3D SGLVector3D::operator-(const SGLVector3D& v) const
{
	return SGLVector3D( x-v.x, y-v.y, z-v.z );
}
const SGLVector3D& SGLVector3D::operator-(const SGLVector3D& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}
void SGLVector3D::operator-=(const SGLVector3D& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
}

//VECTOR3D = VECTOR3D + VECTOR3D
SGLVector3D SGLVector3D::operator+(const SGLVector3D& v) const
{
	return SGLVector3D( x+v.x, y+v.y, z+v.z );
}
const SGLVector3D& SGLVector3D::operator+(const SGLVector3D& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}
void SGLVector3D::operator+=(const SGLVector3D& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
}

//VECTOR3D = VECTOR3D + float
SGLVector3D SGLVector3D::operator+(const float f) const
{
	return SGLVector3D(x+f, y+f, z+f);
}
const SGLVector3D& SGLVector3D::operator+(const float f)
{
	x += f;
	y += f;
	z += f;
	return *this;
}
void SGLVector3D::operator+=(const float f)
{
	x += f;
	y += f;
	z += f;
}

//VECTOR3D = VECTOR3D - float
SGLVector3D SGLVector3D::operator-(const float f) const
{
	return SGLVector3D(x-f, y-f, z-f);
}
const SGLVector3D& SGLVector3D::operator-(const float f)
{
	x -= f;
	y -= f;
	z -= f;
	return *this;
}
void SGLVector3D::operator-=(const float f)
{
	x -= f;
	y -= f;
	z -= f;
}

//VECTOR3D = VECTOR3D * float
SGLVector3D SGLVector3D::operator*(const float f) const
{
	return SGLVector3D(x*f, y*f, z*f);
}
/*const SGLVector3D& SGLVector3D::operator*(const float f)
{
	x *= f;
	y *= f;
	z *= f;
	return *this;
}*/
void SGLVector3D::operator*=(const float f)
{
	x *= f;
	y *= f;
	z *= f;
}