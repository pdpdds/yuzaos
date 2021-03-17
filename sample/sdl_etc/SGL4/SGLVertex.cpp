#include "SGLVertex.h"

SGLVertex::SGLVertex(void)
{
	u = v = 0.0f;
}
SGLVertex::SGLVertex(float x, float y, float z, const SGLColor& c, SGLVector3D& nv, float u, float v)
{
	pos.x = x;
	pos.y = y;
	pos.z = z;
	this->u = u;
	this->v = v;
	this->normal = nv;
	this->color = c;
}
SGLVertex::SGLVertex(float x, float y, float z, float u, float v)
{
	pos.x = x;
	pos.y = y;
	pos.z = z;
	this->u = u;
	this->v = v;
}
SGLVertex::SGLVertex(const SGLPoint3D& pt, const SGLColor& c, SGLVector3D& nv, float U, float V)
{
	pos = pt;
	color = c;
	normal = nv;
	u = U;
	v = V;
}
SGLVertex::SGLVertex(const SGLVertex& vx)
{
	pos = vx.pos;
	u   = vx.u;
	v   = vx.v;
	color=vx.color;
	normal=vx.normal;
}
SGLVertex::~SGLVertex(void)
{
}
void SGLVertex::setTexCoord(float U, float V)
{
	u = U;
	v = V;
}
void SGLVertex::setColor(const SGLColor& c)
{
	color = c;
}
void SGLVertex::setNormal(const SGLVector3D& nv)
{
	normal = nv;
}