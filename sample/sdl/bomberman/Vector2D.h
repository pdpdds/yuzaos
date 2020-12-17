//
//  Vector2D.h
//  SDL Game Programming Book
//
//  Created by shaun mitchell on 27/01/2013.
//  Copyright (c) 2013 shaun mitchell. All rights reserved.
//

#ifndef __SDL_Game_Programming_Book__Vector2D__
#define __SDL_Game_Programming_Book__Vector2D__

#include <math.h>
#include "utils.h"

class Vector2D
{
public:
    Vector2D()
    {
        m_x = 0;
        m_y = 0;
    }
    
    Vector2D(float x, float y): m_x(x), m_y(y) {}
    
	void Zero(){ m_x = 0.0; m_y = 0.0; }

	//returns true if both x and y are zero
	bool isZero()const{ return (m_x*m_x + m_y*m_y) < MinDouble; }

    const float getX() { return m_x; }
    const float getY() { return m_y; }
    
    void setX(float x) { m_x = x; }
    void setY(float y) { m_y = y; }
    
    float Length() { return sqrt(m_x * m_x + m_y * m_y); }
    
    //Vector2D operator+(const Vector2D& v2) const { return Vector2D(m_x + v2.m_x, m_y + v2.m_y); }

	//we need some overloaded operators
	const Vector2D& operator+=(const Vector2D &rhs)
	{
		m_x += rhs.m_x;
		m_y += rhs.m_y;

		return *this;
	}
    
    //Vector2D operator-(const Vector2D& v2) const { return Vector2D(m_x - v2.m_x, m_y - v2.m_y); }
    friend Vector2D& operator-=(Vector2D& v1, const Vector2D& v2)
    {
        v1.m_x -= v2.m_x;
        v1.m_y -= v2.m_y;
        
        return v1;
    }

    
    Vector2D operator*(float scalar)
    {
        return Vector2D(m_x * scalar, m_y * scalar);
    }
    
    Vector2D& operator*=(float scalar)
    {
        m_x *= scalar;
        m_y *= scalar;
        
        return *this;
    }
    
    Vector2D operator/(float scalar)
    {
        return Vector2D(m_x / scalar, m_y / scalar);
    }
    
    Vector2D& operator/=(float scalar)
    {
        m_x /= scalar;
        m_y /= scalar;
        
        return *this;
    }

	bool operator==(const Vector2D& rhs)const
	{
		return (isEqual(m_x, rhs.m_x) && isEqual(m_y, rhs.m_y));
	}

	bool operator!=(const Vector2D& rhs)const
	{
		return (m_x != rhs.m_x) || (m_y != rhs.m_y);
	}


	const Vector2D& operator-=(const Vector2D &rhs)
	{
		m_x -= rhs.m_x;
		m_y -= rhs.m_y;

		return *this;
	}
    
	void Normalize()
    {
		float l = Length();
        if ( l > 0)
        {
            (*this) *= 1 / l;
        }
    }

	Vector2D Perp() const
	{
		return Vector2D(-m_y, m_x);
	}

	//------------------------- LengthSq -------------------------------------
	//
	//  returns the squared length of a 2D vector
	//------------------------------------------------------------------------
	float LengthSq()const
	{
		return (m_x * m_x + m_y * m_y);
	}


	//------------------------- Vec2DDot -------------------------------------
	//
	//  calculates the dot product
	//------------------------------------------------------------------------
	float Dot(const Vector2D &v2)const
	{
		return m_x*v2.m_x + m_y*v2.m_y;
	}

	//------------------------ Sign ------------------------------------------
	//
	//  returns positive if v2 is clockwise of this vector,
	//  minus if anticlockwise (Y axis pointing down, X axis to right)
	//------------------------------------------------------------------------
	enum { clockwise = 1, anticlockwise = -1 };

	int Sign(const Vector2D& v2)const
	{
		if (m_y*v2.m_x > m_x*v2.m_y)
		{
			return anticlockwise;
		}
		else
		{
			return clockwise;
		}
	}

	float Distance(const Vector2D &v2)const
	{
		float ySeparation = v2.m_y - m_y;
		float xSeparation = v2.m_x - m_x;

		return sqrt(ySeparation*ySeparation + xSeparation*xSeparation);
	}

	float DistanceSq(const Vector2D &v2)const
	{
		float ySeparation = v2.m_y - m_y;
		float xSeparation = v2.m_x - m_x;

		return ySeparation*ySeparation + xSeparation*xSeparation;
	}

	//----------------------------- Truncate ---------------------------------
	//
	//  truncates a vector so that its length does not exceed max
	//------------------------------------------------------------------------
	void Truncate(float max)
	{
		if (this->Length() > max)
		{
			this->Normalize();

			*this *= max;
		}
	}

	//--------------------------- Reflect ------------------------------------
	//
	//  given a normalized vector this method reflects the vector it
	//  is operating upon. (like the path of a ball bouncing off a wall)
	//------------------------------------------------------------------------
	inline void Reflect(const Vector2D& norm);

	//----------------------- GetReverse ----------------------------------------
	//
	//  returns the vector that is the reverse of this vector
	//------------------------------------------------------------------------
	Vector2D GetReverse()const
	{
		return Vector2D(-this->m_x, -this->m_y);
	}
    
    float m_x;
    float m_y;
};

inline Vector2D operator*(const Vector2D &lhs, float rhs);
inline Vector2D operator*(float lhs, Vector2D &rhs);
inline Vector2D operator*(double lhs, Vector2D &rhs);
inline Vector2D operator-(const Vector2D &lhs, const Vector2D &rhs);
inline Vector2D operator+(const Vector2D &lhs, const Vector2D &rhs);
inline Vector2D operator/(const Vector2D &lhs, float val);

//------------------------------------------------------------------------operator overloads
inline Vector2D operator*(const Vector2D &lhs, float rhs)
{
	Vector2D result(lhs);
	result.m_x *= rhs;
	result.m_y *= rhs;
	return result;
}

inline Vector2D operator*(double lhs, Vector2D &rhs)
{
	Vector2D result(rhs);
	result.m_x *= lhs;
	result.m_y *= lhs;
	return result;
}

inline Vector2D operator*(float lhs, Vector2D &rhs)
{
	Vector2D result(rhs);
	result *= lhs;
	return result;
}

//overload the - operator
inline Vector2D operator-(const Vector2D &lhs, const Vector2D &rhs)
{
	Vector2D result(lhs);
	result.m_x -= rhs.m_x;
	result.m_y -= rhs.m_y;

	return result;
}

//overload the + operator
inline Vector2D operator+(const Vector2D &lhs, const Vector2D &rhs)
{
	Vector2D result(lhs);
	result.m_x += rhs.m_x;
	result.m_y += rhs.m_y;

	return result;
}

//overload the / operator
inline Vector2D operator/(const Vector2D &lhs, float val)
{
	Vector2D result(lhs);
	result.m_x /= val;
	result.m_y /= val;

	return result;
}

inline void Vector2D::Reflect(const Vector2D& norm)
{
	Vector2D reverse = norm.GetReverse();
	*this += 2.0f * this->Dot(norm) * reverse;
}

inline float Vec2DDistanceSq(const Vector2D &v1, const Vector2D &v2)
{

	float ySeparation = v2.m_y - v1.m_y;
	float xSeparation = v2.m_x - v1.m_x;

	return ySeparation*ySeparation + xSeparation*xSeparation;
}

inline float Vec2DDistance(const Vector2D &v1, const Vector2D &v2)
{

	float ySeparation = v2.m_y - v1.m_y;
	float xSeparation = v2.m_x - v1.m_x;

	return sqrt(ySeparation*ySeparation + xSeparation*xSeparation);
}

inline Vector2D Vec2DNormalize(const Vector2D &v)
{
	Vector2D vec = v;

	float vector_length = vec.Length();

	if (vector_length > std::numeric_limits<float>::epsilon())
	{
		vec.m_x /= vector_length;
		vec.m_y /= vector_length;
	}

	return vec;
}



#endif /* defined(__SDL_Game_Programming_Book__Vector2D__) */
