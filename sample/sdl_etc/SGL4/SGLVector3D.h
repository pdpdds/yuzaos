#ifndef SGLVECTOR3D_H
#define SGLVECTOR3D_H

class SGLVector3D
{
public:
	float x, y, z, w;
public:
	SGLVector3D(void) : x(0.0f), y(0.0f), z(0.0f), w(0.0f)
	{}
	SGLVector3D(const SGLVector3D& v) : x(v.x), y(v.y), z(v.z), w(0.0f)
	{}
	SGLVector3D(float X, float Y, float Z) : x(X), y(Y), z(Z), w(0.0f)
	{}
	~SGLVector3D(void)
	{}

	float length(void) const;
	void  normalize(void);
	float dot(const SGLVector3D& v) const;
	SGLVector3D cross(const SGLVector3D& v) const;

	//VECTOR3D = VECTOR3D - VECTOR3D
	SGLVector3D operator-(const SGLVector3D& v) const;
	const SGLVector3D& operator-(const SGLVector3D& v);
	void operator-=(const SGLVector3D& v);
	
	//VECTOR3D = VECTOR3D + VECTOR3D
	SGLVector3D operator+(const SGLVector3D& v) const;
	const SGLVector3D& operator+(const SGLVector3D& v);
	void operator+=(const SGLVector3D& v);

	//VECTOR3D = VECTOR3D + float
	SGLVector3D operator+(const float f) const;
	const SGLVector3D& operator+(const float f);
	void operator+=(const float f);

	//VECTOR3D = VECTOR3D - float
	SGLVector3D operator-(const float f) const;
	const SGLVector3D& operator-(const float f);
	void operator-=(const float f);
	
	//VECTOR3D = VECTOR3D * float
	SGLVector3D operator*(const float f) const;
	//const SGLVector3D& operator*(const float f);
	void operator*=(const float f);

};


#endif