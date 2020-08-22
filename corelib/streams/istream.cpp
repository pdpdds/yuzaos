#include <istream>
#include <stdio.h>
#include <stdarg.h>

namespace std
{

/**************************************************************/
istream::istream()
/**************************************************************/
{
}

/**************************************************************/
istream::~istream()
/**************************************************************/
{
}

/**************************************************************/
int istream::InFunction(const char* format, ...)
/**************************************************************/
{
	return 0;
	//va_list argList;
	//va_start(argList, format);
	//return(scanf(format, va_arg(argList, double))); //Sloppy, stack manip problems
}

/**************************************************************/
bool istream::eof()
/**************************************************************/
{
	if ((this->rdstate())&ios::eofbit)
		return true;
	else
		return false;
}

/**************************************************************/
istream& istream::getline(char* s, int streamsize, char delimiter)
/**************************************************************/
{
	int ctr = 0;
	char current;
	do
	{
		istream& me = *this;
		me>>current; //get single character off stream
		s[ctr]=current;
		ctr++;

	}
	while (current!=delimiter && (!(eof())) && ctr<streamsize);

	return (*this);
}

/**************************************************************/
//istream& istream::getline(string& s, char delimiter)
/**************************************************************/
/*{
	int ctr = 0;
	char current; //embedded visual c++ 4.0 bug, can't disambiguate between char and char*
	//TODO: This implementation gives bugs when using istringstring or stringstream.getline()
	//because istringstream is stupid
	do
	{
		operator>>(current); //get single character off stream

		s = s + current;
	}
	while (current!=delimiter && (!(eof())));


	return (*this);
}*/

/**************************************************************/
template <typename T> istream& istream::operator>>(T &var)
/**************************************************************/
{
	//I don't like this C++ style template
	//because I'm going to get two copies
	//of a function that do the same thing
	return(istream::operator >>(*this, var));
}

/**************************************************************/
//template <typename T> istream& istream::operator>>(istream& stream, T &var)
/**************************************************************/
//{
	//This is a static method, and my weird way of enabling third party classes
	//to override stream operators...it's not quite the way standard implementation
	//works...but follows the interface.

	//return(operator>>(stream, var)); //The class utilizing stream overloading must
	                                 //have a external overload for this method(standard)
//}

/**************************************************************/
istream& istream::operator >>(char& c)
/**************************************************************/
{
	/*NOTE: If you want to skip spaces, uncomment follow loop*/
//	do
//	{
//		InFunction("%c", &c); //Strange bug, scanf,fscanf, sscanf don't ignore
//		                      //spacing when passing in single char arguments
//	}
//	while (c==' ');

	InFunction("%c", &c); //...all comment out this line if you uncomment the loop
	return (*this);
}

/**************************************************************/
istream& istream::operator >>(char* s)
/**************************************************************/
{
	InFunction("%s", s);
	return (*this);
}

/**************************************************************/
istream& istream::operator >>(const char* s)
/**************************************************************/
{
	InFunction("%s", s);
	return (*this);
}

/**************************************************************/
istream& istream::operator >>(double& d)
/**************************************************************/
{
	InFunction("%Lf", &d);
	return (*this);
}


/**************************************************************/
istream& istream::operator >>(float& f)
/**************************************************************/
{
	InFunction("%f", &f);
	return (*this);
}

/**************************************************************/
istream& istream::operator >>(int& i)
/**************************************************************/
{
	InFunction("%i", &i);
	return (*this);
}


/**************************************************************/
istream& istream::operator >>(long& l)
/**************************************************************/
{
	InFunction("%li", &l);
	return (*this);
}

/**************************************************************/
istream& istream::operator >>(short& s)
/**************************************************************/
{
	InFunction("%hi", &s);
	return (*this);
}

/**************************************************************/
istream& istream::operator >>(unsigned int& ui)
/**************************************************************/
{
	InFunction("%u", &ui);
	return (*this);
}

/**************************************************************/
istream& istream::operator >>(unsigned long& ul)
/**************************************************************/
{
	InFunction("%lu", &ul);
	return (*this);
}

/**************************************************************/
istream& istream::operator >>(unsigned short& us)
/**************************************************************/
{
	InFunction("%hu", &us);
	return (*this);
}

/**************************************************************/
//istream& istream::operator >>(string& s)
/**************************************************************/
/*{
	char buffer[MAX_SIZE_STD_STRING_IN_STREAM]; //The buffer size can't be known, max 2 K
	InFunction("%s", buffer);
	string temp(buffer);
	s = buffer;
	return (*this);
}*/

/**************************************************************/
//istream::operator void*()
/**************************************************************/
/*{
	
	if (!(this->ios::operator void *()))
		return false;

	if (eof())
		return false;
		
	return (void*)true;

}*/

}; /*end namespace ppcStreams*/
