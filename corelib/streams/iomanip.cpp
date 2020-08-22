#include <iomanip>
#include <iostream>

namespace std
{

/********************************************************/
ostream& operator<<(ostream& stream, const setw& value)
/********************************************************/
{
	stream.width(value._width);
	return(stream);
}
/********************************************************/
istream& operator>>(istream& stream, setw& value)
/********************************************************/
{
	stream.width(value._width);
	return(stream);
}
/********************************************************/
ostream& operator<<(ostream& stream, const setprecision& value)
/********************************************************/
{
	stream.precision(value._precision);
	return(stream);
}
/********************************************************/
istream& operator>>(istream& stream, setprecision& value)
/********************************************************/
{
	stream.precision(value._precision);
	return(stream);
}

};