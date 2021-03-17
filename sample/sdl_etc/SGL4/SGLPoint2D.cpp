#include "SGLPoint2D.h"
#include <math.h>

float SGLPoint2D::getLength(const SGLPoint2D& pt) const
{
	return sqrt( float((x-pt.x)*(x-pt.x) + (y-pt.y)*(y-pt.y)) );
}

//pt + pt
SGLPoint2D SGLPoint2D::operator+(const SGLPoint2D& pt) const
{
	return SGLPoint2D(x + pt.x, y + pt.y);
}
/*const SGLPoint2D& SGLPoint2D::operator+(const SGLPoint2D& pt)
{
	x += pt.x;
	y += pt.y;
	return *this;
}*/
void SGLPoint2D::operator+=(const SGLPoint2D& pt)
{
	x += pt.x;
	y += pt.y;
}

//pt + int
SGLPoint2D SGLPoint2D::operator+(const int f) const
{
	return SGLPoint2D(x + f, y + f);
}
/*const SGLPoint2D& SGLPoint2D::operator+(const int f)
{
	x += f;
	y += f;
	return *this;
}*/
void SGLPoint2D::operator+=(const int f)
{
	x += f;
	y += f;
}

//pt - pt
SGLPoint2D SGLPoint2D::operator-(const SGLPoint2D& pt) const
{
	return SGLPoint2D(x - pt.x, y - pt.y);
}
/*const SGLPoint2D& SGLPoint2D::operator-(const SGLPoint2D& pt)
{
	x -= pt.x;
	y -= pt.y;
	return *this;
}*/
void SGLPoint2D::operator-=(const SGLPoint2D& pt)
{
	x -= pt.x;
	y -= pt.y;
}

//pt - int
SGLPoint2D SGLPoint2D::operator-(const int f) const
{
	return SGLPoint2D(x - f, y - f);
}
/*const SGLPoint2D& SGLPoint2D::operator-(const int f)
{
	x -= f;
	y -= f;
	return *this;
}*/
void SGLPoint2D::operator-=(const int f)
{
	x -= f;
	y -= f;
}


//pt * f
SGLPoint2D SGLPoint2D::operator*(const int f) const
{
	return SGLPoint2D(x*f, y*f);
}
/*const SGLPoint2D& SGLPoint2D::operator*(const int f)
{
	x *= f;
	y *= f;
	return *this;
}*/
void SGLPoint2D::operator*=(const int f)
{
	x *= f;
	y *= f;
}