#include <ios_base>

//namespace eastl
//{

//Sigh...another problem with eVC 4.0, initialization seperation from declaration

const char ios_base::app    = 0x01;
const char ios_base::ate    = 0x02;
const char ios_base::binary = 0x04;
const char ios_base::in     = 0x08;
const char ios_base::out    = 0x10;
const char ios_base::trunc  = 0x20;

const int ios_base::boolalpha = 0x0001;
const int ios_base::dec = 0x0002;
const int ios_base::fixed = 0x0004;
const int ios_base::hex = 0x0008;
const int ios_base::internal = 0x0010;
const int ios_base::left = 0x0020;
const int ios_base::right = 0x0040; 
const int ios_base::oct = 0x0080;
const int ios_base::scientific = 0x0100;
const int ios_base::showbase = 0x0200;
const int ios_base::showpoint = 0x0400;
const int ios_base::showpos = 0x0800;
const int ios_base::skipws = 0x1000;
const int ios_base::unitbuf = 0x2000;
const int ios_base::upperbuf = 0x4000;
const int ios_base::uppercase = 0x8000;

const char ios_base::badbit  = 0x01;
const char ios_base::eofbit  = 0x02;
const char ios_base::failbit = 0x04;
const char ios_base::goodbit = 0x08;


/**************************************************************/
ios_base::ios_base()
/**************************************************************/
{
	_iostate = goodbit;
	_precision = 10;
}

/**************************************************************/
ios_base::~ios_base()
/**************************************************************/
{
}

/**************************************************************/
bool ios_base::good()
/**************************************************************/
{
	if((goodbit)& _iostate)
		return true;
	else
		return false;
}

/**************************************************************/
bool ios_base::bad()
/**************************************************************/
{
	if(this->badbit&_iostate)
		return true;
	else
		return false;
}

/**************************************************************/
streamsize ios_base::precision() const
/**************************************************************/
{
	return (_precision);
}

/**************************************************************/
streamsize ios_base::precision(streamsize prec)
/**************************************************************/
{
	_precision = prec;
	return (_precision);
}

/**************************************************************/
streamsize ios_base::width() const
/**************************************************************/
{
	return (_width);
}

/**************************************************************/
streamsize ios_base::width(streamsize width)
/**************************************************************/
{
	_width = width;
	return (_width);
}

/**************************************************************/
void ios_base::setf(fmtflags flag)
/**************************************************************/
{
	_fmtflags = flag;
}

//}; /*end namespace ppcStreams*/
