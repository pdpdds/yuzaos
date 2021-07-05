#include <ostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

namespace std
{

/**************************************************************/
ostream::ostream()
/**************************************************************/
{

}

/**************************************************************/
ostream::~ostream()
/**************************************************************/
{
}

/**************************************************************/
int ostream::OutFunction(const char* format, ...)
/**************************************************************/
{
	va_list argList;
	va_start(argList, format);
	printf(format, va_arg(argList, double));
	return 0; //Sloppy, stack manip problems
}

/**************************************************************/
ostream& ostream::operator<<(ostream& (*__pf)(ostream&))
/**************************************************************/
{
	(*this)<<(void*)__pf;
	return(*this);
	//const ostream& hoohah = __pf(*this);
	//return ((ostream&)hoohah);
}

/**************************************************************/
ostream& ostream::operator <<(void* c)
/**************************************************************/
{
	OutFunction("%i", c);
	return (*this);
}

/**************************************************************/
ostream& ostream::operator <<(char c)
/**************************************************************/
{
	OutFunction("%c", c);
	return (*this);
}


/**************************************************************/
ostream& ostream::operator <<(char* s)
/**************************************************************/
{
	OutFunction("%s", s);
	return (*this);
}

/**************************************************************/
ostream& ostream::operator <<(const char* s)
/**************************************************************/
{
	OutFunction("%s", s);
	return (*this);
}

/**************************************************************/
ostream& ostream::operator <<(double d)
/**************************************************************/
{
	OutFunction("%Lf", d);
	return (*this);
}

/**************************************************************/
ostream& ostream::operator <<(float f)
/**************************************************************/
{
	OutFunction("%f", f);
	return (*this);
}

/**************************************************************/
ostream& ostream::operator <<(int i)
/**************************************************************/
{
	OutFunction("%i", i);
	return (*this);
}

/**************************************************************/
ostream& ostream::operator <<(long l)
/**************************************************************/
{
	OutFunction("%li", l);
	return (*this);
}

/**************************************************************/
ostream& ostream::operator <<(short s)
/**************************************************************/
{
	OutFunction("%hi", s);
	return (*this);
}

/**************************************************************/
ostream& ostream::operator <<(unsigned int ui)
/**************************************************************/
{
	OutFunction("%u", ui);
	return (*this);
}

/**************************************************************/
ostream& ostream::operator <<(unsigned long ul)
/**************************************************************/
{
	OutFunction("%lu", ul);
	return (*this);
}

/**************************************************************/
ostream& ostream::operator <<(unsigned short us)
/**************************************************************/
{
	OutFunction("%hu", us);
	return (*this);
}

int ostream::write(const char* s, int count)
{
	if (count <= 0)
		return 0;

	int ctr = 0;	
	do
	{
		ostream& me = *this;
		me << s[ctr];		
		ctr++;

	} while (ctr < count);

	return ctr;
}

/**************************************************************/
//ostream& ostream::operator <<(string s)
/**************************************************************/
//{
	//OutFunction("%s", s.c_str());
	//return (*this);
//}

}; /*End namespace ppcStreams*/