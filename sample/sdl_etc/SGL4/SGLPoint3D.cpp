#include "SGLPoint3D.h"
#include <math.h>
float SGLPoint3D::getLength(const SGLPoint3D& pt) const
{
	return sqrt( float( (x-pt.x)*(x-pt.x) + (y-pt.y)*(y-pt.y) + (z-pt.z)*(z-pt.z) ) );
}
float SGLPoint3D::getLength(void) const
{
	return sqrt( float( (x*x) + (y*y) + (z*z) ) );
}
//VECTOR3D = PT3D - PT3D
SGLVector3D SGLPoint3D::operator-(const SGLPoint3D& pt) const
{
	return SGLVector3D(x-pt.x, y-pt.y, z-pt.z);
}

//pt + pt
SGLPoint3D SGLPoint3D::operator+(const SGLPoint3D& pt) const
{
	return SGLPoint3D(x+pt.x, y+pt.y, z+pt.z);
}
/*const SGLPoint3D& SGLPoint3D::operator+(const SGLPoint3D& pt)
{
	x += pt.x;
	y += pt.y;
	z += pt.z;
	return *this;
}*/
void SGLPoint3D::operator+=(const SGLPoint3D& pt)
{
	x += pt.x;
	y += pt.y;
	z += pt.z;
}

//pt + float
SGLPoint3D SGLPoint3D::operator+(const float f) const
{
	return SGLPoint3D(x+f, y+f, z+f);
}
/*const SGLPoint3D& SGLPoint3D::operator+(const float f)
{
	x += f;
	y += f;
	z += f;
	return *this;
}*/
void SGLPoint3D::operator+=(const float f)
{
	x += f;
	y += f;
	z += f;
}



//pt - float
SGLPoint3D SGLPoint3D::operator-(const float f) const
{
	return SGLPoint3D(x-f, y-f, z-f);
}
/*const SGLPoint3D& SGLPoint3D::operator-(const float f)
{
	x -= f;
	y -= f;
	z -= f;
	return *this;
}*/
void SGLPoint3D::operator-=(const float f)
{
	x -= f;
	y -= f;
	z -= f;
}

//pt * float
SGLPoint3D SGLPoint3D::operator*(const float f) const
{
	return SGLPoint3D(x*f, y*f, z*f);
}
/*const SGLPoint3D& SGLPoint3D::operator*(const float f)
{
	x *= f;
	y *= f;
	z *= f;
	return *this;
}*/
void SGLPoint3D::operator*=(const float f)
{
	x *= f;
	y *= f;
	z *= f;
}