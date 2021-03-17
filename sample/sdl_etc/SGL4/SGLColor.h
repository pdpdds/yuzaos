#ifndef SGLCOLOR_H
#define SGLCOLOR_H

class SGLColor
{
public:
	unsigned char R;
	unsigned char G;
	unsigned char B;
	unsigned char A;
public:
	SGLColor(void) : R(0), G(0), B(0), A(255)
	{}
	SGLColor(const SGLColor& color) : R(color.R), G(color.G), B(color.B), A(color.A)
	{}
	SGLColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a=255) 
		: R(r), G(g), B(b), A(a)
	{}
	~SGLColor(void)
	{}
	void setAlpha(unsigned char a);
	void plusAlpha(unsigned char a);
	void minusAlpha(unsigned char a);
	void mulAlpha(float a);

	SGLColor operator*(float f) const;
	//const SGLColor& operator*(float f);
	void operator*=(float f);

	SGLColor operator/(float f) const;
	//const SGLColor& operator*(float f);
	void operator/=(float f);

	SGLColor operator+(unsigned char f) const;
	//const SGLColor& operator+(unsigned char f);
	void operator+=(unsigned char f);

	SGLColor operator+(const SGLColor& color) const;
	//const SGLColor& operator+(const SGLColor& color);
	void operator+=(const SGLColor& color);

	SGLColor operator-(unsigned char f) const;
	//const SGLColor& operator-(unsigned char f);
	void operator-=(unsigned char f);

	SGLColor operator-(const SGLColor& color) const;
	//const SGLColor& operator-(const SGLColor& color);
	void operator-=(const SGLColor& color);
	
};

#endif