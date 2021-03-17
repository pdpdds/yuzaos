#ifndef SGLPOINT2D_H
#define SGLPOINT2D_H

class SGLPoint2D
{
public:
	unsigned int x;
	unsigned int y;
public:
	SGLPoint2D(void) : x(0), y(0)
	{}
	SGLPoint2D(const SGLPoint2D& pt) : x(pt.x), y(pt.y)
	{}
	SGLPoint2D(const int X, const int Y) : x(X), y(Y)
	{}
	~SGLPoint2D(void)
	{}
	float getLength(const SGLPoint2D& pt) const;

	//pt + pt
	SGLPoint2D operator+(const SGLPoint2D& pt) const;
	//const SGLPoint2D& operator+(const SGLPoint2D& pt);
	void operator+=(const SGLPoint2D& pt);

	//pt + int
	SGLPoint2D operator+(const int f) const;
	//const SGLPoint2D& operator+(const int f);
	void operator+=(const int f);

	//pt - pt
	SGLPoint2D operator-(const SGLPoint2D& pt) const;
	//const SGLPoint2D& operator-(const SGLPoint2D& pt);
	void operator-=(const SGLPoint2D& pt);

	//pt - int
	SGLPoint2D operator-(const int f) const;
	//const SGLPoint2D& operator-(const int f);
	void operator-=(const int f);
	
	//pt * f
	SGLPoint2D operator*(const int f) const;
	//const SGLPoint2D& operator*(const int f);
	void operator*=(const int f);
};

#endif
