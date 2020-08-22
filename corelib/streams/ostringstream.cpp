#include <ostringstream>

namespace std
{

/**************************************************************/
ostringstream::ostringstream(ios::openmode mode)
/**************************************************************/
{
	str("");
}

/**************************************************************/
ostringstream::ostringstream(string output, ios::openmode mode)
/**************************************************************/
{
	str(output);
}

/**************************************************************/
ostringstream::~ostringstream()
/**************************************************************/
{
}

/**************************************************************/
int ostringstream::OutFunction(const char* format, ...)
/**************************************************************/
{
	va_list argList;
	va_start(argList, format);
	char temp[MAX_SIZE_STD_STRING_IN_STREAM]; //limit size of single argument to 2048 byte (2 K)
	sprintf(temp, format, va_arg(argList, double));
	size_t extraBytesNeeded = strlen(temp);
	char* temp2 = new char[str().length()+1+(int)extraBytesNeeded+1];
	strcpy(temp2, str().c_str());
	strcat(temp2, temp);
	str(string(temp2));
	delete(temp2);
	return (int)extraBytesNeeded;
}

/**************************************************************/
string ostringstream::str()
/**************************************************************/
{
	return(_output);
}

/**************************************************************/
void ostringstream::str(string val)
/**************************************************************/
{
	_output = val;
}

}; /*end namespace ppcStreams*/