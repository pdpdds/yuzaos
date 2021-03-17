#include "SGLColor.h"

void SGLColor::setAlpha(unsigned char a)
{
	A = a;
}
void SGLColor::plusAlpha(unsigned char a)
{
	A += a;
}
void SGLColor::minusAlpha(unsigned char a)
{
	A -= a;
}
void SGLColor::mulAlpha(float a)
{
	A *= a;
}
SGLColor SGLColor::operator*(float f) const
{
	return SGLColor(R*f, G*f, B*f, A);
}
/*
const SGLColor& SGLColor::operator*(float f)
{
	float r = (((float)R*f) + 0.5f);
	float g = (((float)G*f) + 0.5f);
	float b = (((float)B*f) + 0.5f);
	if( r > 255 )
		R = 255;
	else 
		R = r;

	if( g > 255 )
		G = 255;
	else 
		G = g;

	if( b > 255 )
		B = 255;
	else 
		B = b;

	return *this;
}
*/
void SGLColor::operator*=(float f)
{
	float r = (((float)R*f) + 0.5f);
	float g = (((float)G*f) + 0.5f);
	float b = (((float)B*f) + 0.5f);
	if( r >= 255 )
		R = 255;
	else
	if( r <= 0)
		R = 0;
	else 
		R = r;


	if( g >= 255 )
		G = 255;
	else
	if( g <= 0)
		G = 0;
	else 
		G = g;

	if( b >= 255 )
		B = 255;
	else
	if( b <= 0)
		B = 0;
	else 
		B = b;
}

SGLColor SGLColor::operator/(float f) const
{
	return SGLColor(R/f, G/f, B/f, A);
}

void SGLColor::operator/=(float f)
{
	float r = (((float)R/f) + 0.5f);
	float g = (((float)G/f) + 0.5f);
	float b = (((float)B/f) + 0.5f);
	if( r >= 255 )
		R = 255;
	else 
		R = r;

	if( g >= 255 )
		G = 255;
	else 
		G = g;

	if( b >= 255 )
		B = 255;
	else 
		B = b;
}

SGLColor SGLColor::operator+(unsigned char f) const
{
	SGLColor c;
	if(R+f >= 255)
		c.R = 255;
	else
		c.R = R+f;

	if(G+f >= 255)
		c.G = 255;
	else
		c.G = G+f;

	if(B+f >= 255)
		c.B = 255;
	else
		c.B = B+f;
	return c; 
}
/*const SGLColor& SGLColor::operator+(unsigned char f)
{
	R += f;
	G += f;
	B += f;
	return *this;
}*/
void SGLColor::operator+=(unsigned char f)
{
	if(R+f >= 255)
		R = 255;
	else
		R = R+f;

	if(G+f >= 255)
		G = 255;
	else
		G = G+f;

	if(B+f >= 255)
		B = 255;
	else
		B = B+f;
}

SGLColor SGLColor::operator+(const SGLColor& c) const
{
	SGLColor cc;
	if(R+c.R >= 255)
		cc.R = 255;
	else
		cc.R = R+c.R;

	if(G+c.G >= 255)
		cc.G = 255;
	else
		cc.G = G+c.G;

	if(B+c.B >= 255)
		cc.B = 255;
	else
		cc.B = B+c.B;
	return cc;
}
/*const SGLColor& SGLColor::operator+(const SGLColor& c)
{
	R += c.R;
	G += c.G;
	B += c.B;
	return *this;
}*/
void SGLColor::operator+=(const SGLColor& c)
{
	if(R+c.R >= 255)
		R = 255;
	else
		R = c.R;

	if(G+c.G >= 255)
		G = 255;
	else
		G = c.G;

	if(B+c.B >= 255)
		B = 255;
	else
		B = c.B;
}

SGLColor SGLColor::operator-(unsigned char f) const
{
	SGLColor c;
	if(R-f <= 0)
		c.R = 0;
	else
		c.R = R-f;

	if(G-f <= 0)
		c.G = 0;
	else
		c.G = G-f;

	if(B-f <= 0)
		c.B = 0;
	else
		c.B = B-f;

	
	return c;
}
/*const SGLColor& SGLColor::operator-(unsigned char f)
{
	R -= f;
	G -= f;
	B -= f;
	return *this;
}*/
void SGLColor::operator-=(unsigned char f)
{
	SGLColor c;
	if(R-f <= 0)
		c.R = 0;
	else
		c.R = R-f;

	if(G-f <= 0)
		c.G = 0;
	else
		c.G = G-f;

	if(B-f <= 0)
		c.B = 0;
	else
		c.B = B-f;
	*this = c;
}

SGLColor SGLColor::operator-(const SGLColor& c) const
{
	SGLColor cc;
	if(R-c.R <= 0)
		cc.R = 0;
	else
		cc.R = R-c.R;

	if(G-c.G <= 0)
		cc.G = 0;
	else
		cc.G = G-c.G;

	if(B-c.B <= 0)
		cc.B = 0;
	else
		cc.B = B-c.B;
	return cc;
}
/*const SGLColor& SGLColor::operator-(const SGLColor& c)
{
	R -= c.R;
	G -= c.G;
	B -= c.B;
	return *this;
}*/
void SGLColor::operator-=(const SGLColor& c)
{
	SGLColor cc;
	if(R-c.R <= 0)
		cc.R = 0;
	else
		cc.R = R-c.R;

	if(G-c.G <= 0)
		cc.G = 0;
	else
		cc.G = G-c.G;

	if(B-c.B <= 0)
		cc.B = 0;
	else
		cc.B = B-c.B;
	*this = cc;
}