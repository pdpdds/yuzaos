///////////////////////////////////////////////////////////////////////////////
//
// MathDefs.h : Math Structure Header File
//
// Purpose:	Declare Basic Math Structures
//
// I DIDN'T PUT THESE IN A C++ CLASS FOR CROSS PLATFORM COMPATIBILITY
// SINCE THE ENGINE MAY BE IMPLEMENTED ON CONSOLES AND OTHER SYSTEMS
//
// Created:
//		JL 9/1/97		
// Revisions:
//		Integrated into Kine Demo		8/18/98
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1998 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////

#if !defined(MATHDEFS_H__INCLUDED_)
#define MATHDEFS_H__INCLUDED_

#define M_PI			3.1415926
#define HALF_PI			1.5707963
#define PI_TIMES_TWO	6.2831852
/// Trig Macros ///////////////////////////////////////////////////////////////
#define DEGTORAD(A)	((A * M_PI) / 180.0f)
#define RADTODEG(A)	((A * 180.0f) / M_PI)
#define MAX(a,b) ((a > b) ? a : b)
#define MIN(a,b)   ((a < b) ? a : b)
///////////////////////////////////////////////////////////////////////////////
typedef unsigned char uchar;

typedef int		BOOL;
typedef unsigned int	uint;
typedef unsigned short  ushort;
typedef unsigned char	byte;

typedef struct
{
	float u,v;
} t2DCoord;

struct tVector  
{
	float x,y,z,w;
public:
	tVector() {	x = y = z = 0.0f; w = 1.0f; }	
	tVector(tVector &_v) { x = _v.x; y = _v.y; z = _v.z; w = _v.w; }
	tVector(float _x, float _y, float _z, float _w = 1.0f) { x = _x; y = _y; z = _z; w = _w; }
	~tVector() {}

public:
	
	void	Zero() { x = 0.0f; y = 0.0f; z = 0.0f; w = 0.0f; }
	void	CrossProduct(tVector &v1, tVector &v2);
	float	Length2();
	float	Length(); 
	void	NormalizeVector();
	void	Lerp(tVector *v,float factor);
	float	Dot(tVector *v);
	float	Dot(tVector &v);
	float	Dot(float vx, float vy, float vz, float vw);
	void	MultVectorByMatrix(float *mat);
	void	MultVectorByRotMatrix(float *mat);
	tVector operator+( tVector &arg ) { tVector ret(x + arg.x, y + arg.y, z + arg.z, w + arg.w); return ret; }
	tVector operator-( tVector &arg ) { tVector ret(x - arg.x, y - arg.y, z - arg.z, w - arg.w); return ret; }
	tVector operator=( tVector &arg ) { x = arg.x; y = arg.y; z = arg.z; w = arg.w; return *this; }
	tVector operator*( tVector &arg ) { tVector ret(x * arg.x, y * arg.y, z * arg.z, w * arg.w); return ret; }
	tVector operator=( float *arg );
	tVector operator+=( tVector &arg );
	tVector operator-=( tVector &arg );
	tVector operator*( float arg );
	tVector operator/( float arg );
	tVector operator*=( float arg );
	tVector operator/=( float arg );
};

inline float		DotProd(tVector &v1, tVector &v2)	{ tVector v = v1 * v2; return v.x + v.y + v.z + v.w;	}
inline float		Length2(tVector &v)							{ return DotProd(v, v);			}
inline float		Length(tVector &v)							{ return sqrtf(DotProd(v, v));	}
inline float		rLength(tVector &v)							{ return 1.0f / Length(v);	}
inline tVector		Normalize(tVector &v)						{ return (Length2(v) > 0.0f) ? v * rLength(v) : v;		}

struct tVector3  
{
	float x,y,z;
public:
	tVector3() {	x = y = z = 0; } 
	tVector3(float _x, float _y, float _z) { x = _x; y = _y; z = _z; }
	tVector3(tVector3&_v) { x = _v.x; y = _v.y; z = _v.z; }
	~tVector3() {}

public:
	void	Zero() { x = 0.0f; y = 0.0f; z = 0.0f; }
	void	CrossProduct(tVector3 &v1, tVector3 &v2);
	float	Length2();
	float	Length(); 
	void	NormalizeVector();
	void	Lerp(tVector3 *v,float factor);
	float	Dot(tVector3 *v);
	float	Dot(tVector3 &v);
	float	Dot(float vx, float vy, float vz);
	void	MultVectorByMatrix(float *mat);
	void	MultVectorByRotMatrix(float *mat);
	tVector3 operator+( tVector3 &arg ) { tVector3 ret(x + arg.x, y + arg.y, z + arg.z); return ret; }
	tVector3 operator-( tVector3 &arg ) { tVector3 ret(x - arg.x, y - arg.y, z - arg.z); return ret; }
	tVector3 operator-( tVector &arg ) { tVector3 ret(x - arg.x, y - arg.y, z - arg.z); return ret; }
	tVector3 operator=( tVector3 &arg ) { x = arg.x; y = arg.y; z = arg.z; return *this; }
	tVector3 operator=( tVector &arg ) { x = arg.x; y = arg.y; z = arg.z; return *this; }
	tVector3 operator*( tVector3 &arg ) { tVector3 ret(x * arg.x, y * arg.y, z * arg.z); return ret; }
	tVector3 operator=( float *arg );
	tVector3 operator+=( tVector3 &arg );
	tVector3 operator-=( tVector3 &arg );
	tVector3 operator*( float arg );
	tVector3 operator/( float arg );
	tVector3 operator*=( float arg );
	tVector3 operator/=( float arg );
	tVector3 operator-() { return *this * -1.0f;}
};

/// Quaternion Definitions ////////////////////////////////////////////////////
struct tQuaternion : public tVector
{
	tQuaternion() : tVector() { } 
};
///////////////////////////////////////////////////////////////////////////////

tVector3			CrossProduct(tVector3 &v1, tVector3 &v2);
inline float		DotProd(tVector3 &v1, tVector3 &v2)	{ tVector3 v = v1 * v2; return v.x + v.y + v.z;	}
inline float		Length2(tVector3 &v)				{ return DotProd(v, v);			}
inline float		Length(tVector3 &v)					{ return sqrtf(DotProd(v, v));	}
inline float		rLength(tVector3 &v)				{ return 1.0f / Length(v);	}
inline tVector3		Normalize(tVector3 &v)				{ return (Length2(v) > 0.0f) ? v * rLength(v) : v;		}

typedef struct
{
	float r;
	float g;
	float b;
	float a;
} tColor;

// NOT DECLARED AS float[4][4] BECAUSE OPENGL ACCESSES THIS STRANGLY
typedef struct
{
	float m[16];
} tMatrix;

// SOME STRUCTURES TO HELP ME ACCESS VERTEX DATA IN AN ARRAY
typedef struct
{
	float r,g,b;
	float x,y,z;
} tColoredVertex;

typedef struct
{
	float u,v;
	float x,y,z;
} tTexturedVertex;

typedef struct
{
	float u,v;
	float r,g,b;
	float x,y,z;
} tTexturedColoredVertex;

#define MAKEVECTOR(a,vx,vy,vz)	a.x = vx; a.y = vy; a.z = vz;

void	IdentityMatrix(tMatrix *mat);

#endif // !defined(MATH_H__INCLUDED_)

