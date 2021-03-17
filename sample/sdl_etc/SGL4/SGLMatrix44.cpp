#include "SGLMatrix44.h"
#include "SGLDevice.h"
SGLMatrix44::SGLMatrix44(void)
{
	memset(this, 0, sizeof(SGLMatrix44));
}
SGLMatrix44::SGLMatrix44(const SGLMatrix44& matrix)
{
	for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
			m[i][j] = matrix.m[i][j];

}
SGLMatrix44::~SGLMatrix44(void)
{
}

void SGLMatrix44::identity(void)
{
	memset(this, 0, sizeof(SGLMatrix44));
	m11 = m22 = m33 = m44 = 1.0f;
}
void SGLMatrix44::transpose(void)
{
	SGLMatrix44 t;

	t.m11 = r1[0];
	t.m12 = r1[1];
	t.m13 = r1[2];
	t.m14 = r1[3];

	t.m21 = r2[0];
	t.m22 = r2[1];
	t.m23 = r2[2];
	t.m24 = r2[3];

	t.m31 = r3[0];
	t.m32 = r3[1];
	t.m33 = r3[2];
	t.m34 = r3[3];

	t.m41 = r4[0];
	t.m42 = r4[1];
	t.m43 = r4[2];
	t.m44 = r4[3];

	*this = t;
}

void SGLMatrix44::rotateX(float angle)
{
	identity();
	float COS = sglDevice.COS[((int)angle*10)%3600];
	float SIN = sglDevice.SIN[((int)angle*10)%3600];

	m22 = COS;
	m23 = -SIN;
	m32 = SIN;
	m33 = COS;
}
void SGLMatrix44::rotateY(float angle)
{
	identity();
	float COS = sglDevice.COS[((int)angle*10)%3600];
	float SIN = sglDevice.SIN[((int)angle*10)%3600];

	m21 = COS;
	m23 = SIN;
	m31 = -SIN;
	m33 = COS;
}
void SGLMatrix44::rotateZ(float angle)
{
	identity();
	float COS = sglDevice.COS[((int)angle*10)%3600];
	float SIN = sglDevice.SIN[((int)angle*10)%3600];

	m11 = COS;
	m12 = -SIN;
	m21 = SIN;
	m22 = COS;
}
void SGLMatrix44::rotateXYZ(float x, float y, float z, float angle)
{
	identity();
	SGLVector3D v(x, y, z);
	v.normalize();

	float C = sglDevice.COS[((int)angle*10)%3600];
	float S = sglDevice.SIN[((int)angle*10)%3600];

	m11 = v.x*v.x*(1-C)+C;
	m12 = v.x*v.y*(1-C)-v.z*S;
	m13 = v.z*v.x*(1-C)+v.y*S;
	m14 = 0.0f;

	m21 = v.x*v.y*(1-C)+v.z*S;
	m22 = v.y*v.y*(1-C)+C;
	m23 = v.y*v.z*(1-C)-v.x*S;
	m24 = 0.0f;

	m31 = v.z*v.x*(1-C)-v.y*S;
	m32 = v.y*v.z*(1-C)+v.x*S;
	m33 = v.z*v.z*(1-C)+C;
	m34 = 0.0f;

	m41 = 0.0f;
	m42 = 0.0f;
	m43 = 0.0f;
	m44 = 1.0f;
}
void SGLMatrix44::translate(float x, float y, float z)
{
	identity();
	m14 = x;
	m24 = y;
	m34 = z;
}
void SGLMatrix44::scale(float x, float y, float z)
{
	identity();
	m11 = x;
	m22 = y;
	m33 = z;
}

//accum
void SGLMatrix44::accumRotateX(float angle)
{
	SGLMatrix44 tm;
	tm.rotateX(angle);
	*this *= tm;
}
void SGLMatrix44::accumRotateY(float angle)
{
	SGLMatrix44 tm;
	tm.rotateY(angle);
	*this *= tm;
}
void SGLMatrix44::accumRotateZ(float angle)
{
	SGLMatrix44 tm;
	tm.rotateZ(angle);
	*this *= tm;
}
void SGLMatrix44::accumRotateXYZ(float x, float y, float z, float angle)
{
	SGLMatrix44 tm;
	tm.rotateXYZ(x, y, z, angle);
	*this *= tm;
}
void SGLMatrix44::accumTranslate(float x, float y, float z)
{
	SGLMatrix44 tm;
	tm.translate(x, y, z);
	*this *= tm;
}
void SGLMatrix44::accumScale(float x, float y, float z)
{
	SGLMatrix44 tm;
	tm.scale(x, y, z);
	*this *= tm;
}

//MATRIX44 = MATRIX44 + MATRIX44
SGLMatrix44 SGLMatrix44::operator+(const SGLMatrix44& matrix) const
{
	SGLMatrix44 t;
	int i, j=0;
	for(i=0; i<4; i++)
	{
		for(j=0; j<4; j++)
		{
			t.m[i][j] = m[i][j] + matrix.m[i][j];
		}
	}
	return t;
}
/*const SGLMatrix44& SGLMatrix44::operator+(const SGLMatrix44& matrix)
{
	int i, j=0;
	for(i=0; i<4; i++)
	{
		for(j=0; j<4; j++)
		{
			m[i][j] += matrix.m[i][j];
		}
	}
	return *this;
}*/
void SGLMatrix44::operator+=(const SGLMatrix44& matrix)
{
	int i, j=0;
	for(i=0; i<4; i++)
	{
		for(j=0; j<4; j++)
		{
			m[i][j] += matrix.m[i][j];
		}
	}
}

//MATRIX44 = MATRIX44 - MATRIX44
SGLMatrix44 SGLMatrix44::operator-(const SGLMatrix44& matrix) const
{
	SGLMatrix44 t;
	int i, j=0;
	for(i=0; i<4; i++)
	{
		for(j=0; j<4; j++)
		{
			t.m[i][j] = m[i][j] - matrix.m[i][j];
		}
	}
	return t;
}
/*const SGLMatrix44& SGLMatrix44::operator-(const SGLMatrix44& matrix)
{
	int i, j=0;
	for(i=0; i<4; i++)
	{
		for(j=0; j<4; j++)
		{
			m[i][j] -= matrix.m[i][j];
		}
	}
	return *this;
}*/
void SGLMatrix44::operator-=(const SGLMatrix44& matrix)
{
	int i, j=0;
	for(i=0; i<4; i++)
	{
		for(j=0; j<4; j++)
		{
			m[i][j] -= matrix.m[i][j];
		}
	}
}

//MATRIX44 = MATRIX44 * MATRIX44
SGLMatrix44 SGLMatrix44::operator*(const SGLMatrix44& matrix) const
{
	SGLMatrix44 t;
	int i, j, k;
	for(i=0; i<4; i++)
	{
		for(j=0; j<4; j++)
		{
			t.m[i][j] = 0;
			for(k=0; k<4; k++)
			{
				t.m[i][j] += m[i][k] * matrix.m[k][j];
			}
		}
	}
	return t;
}
/*const SGLMatrix44& SGLMatrix44::operator*(const SGLMatrix44& matrix)
{
	SGLMatrix44 t;
	int i, j, k;
	for(i=0; i<4; i++)
	{
		for(j=0; j<4; j++)
		{
			t.m[i][j] = 0;
			for(k=0; k<4; k++)
			{
				t.m[i][j] += m[i][k] * matrix.m[k][j];
			}
		}
	}
	*this = t;
	return *this;
}*/
void SGLMatrix44::operator*=(const SGLMatrix44& matrix)
{
	SGLMatrix44 t;
	int i, j, k;
	for(i=0; i<4; i++)
	{
		for(j=0; j<4; j++)
		{
			t.m[i][j] = 0;
			for(k=0; k<4; k++)
			{
				t.m[i][j] += m[i][k] * matrix.m[k][j];
			}
		}
	}
	*this = t;
}

//VECTOR3D = MATRIX44 * VECTOR3D
SGLVector3D SGLMatrix44::operator*(const SGLVector3D& v) const
{

	return SGLVector3D(
		m11*v.x + m12*v.y + m13*v.z + m14*v.w,
		m21*v.x + m22*v.y + m23*v.z + m24*v.w,
		m31*v.x + m32*v.y + m33*v.z + m34*v.w
		);
}

//POINT3D = MATRIX44 * POINT3D
SGLPoint3D SGLMatrix44::operator*(const SGLPoint3D& pt) const
{
	SGLPoint3D out_p;
	out_p.x = m11*pt.x + m12*pt.y + m13*pt.z + m14*pt.w;
	out_p.y = m21*pt.x + m22*pt.y + m23*pt.z + m24*pt.w;
	out_p.z = m31*pt.x + m32*pt.y + m33*pt.z + m34*pt.w;
	out_p.w = m41*pt.x + m42*pt.y + m43*pt.z + m44*pt.w;
	out_p.x /= out_p.w;
	out_p.y /= out_p.w;
	out_p.z /= out_p.w;
	out_p.w /= out_p.w;
	return out_p;
}