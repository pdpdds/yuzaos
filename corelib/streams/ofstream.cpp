#include <ofstream>
#include <stdio.h>
#include <stdarg.h>

namespace std
{

/**************************************************************/
ofstream::ofstream()
/**************************************************************/
{
	_fileHandle = NULL;
}

/**************************************************************/
ofstream::ofstream(const char* filename)
/**************************************************************/
{
	open(filename);
}

/**************************************************************/
ofstream::~ofstream()
/**************************************************************/
{
}

/**************************************************************/
int ofstream::OutFunction(const char* format, ...)
/**************************************************************/
{
	va_list argList;
	va_start(argList, format);
	return(vfprintf(_fileHandle, format, argList)); //Sloppy, stack manip problems
}

/**************************************************************/
int ofstream::write(const char* src, int srcSize)
/**************************************************************/
{
	return fwrite(src, srcSize, 1, _fileHandle);
}

/**************************************************************/
bool ofstream::operator==(bool val)
/**************************************************************/
{
	return(is_open()==val);
}

/**************************************************************/
ofstream::operator void*()
/**************************************************************/
{
	if (is_open())
		return ((void*)this);
	else
		return NULL;
}

/**************************************************************/
int ofstream::eof()
/**************************************************************/
{
	return(feof(_fileHandle));
}

/**************************************************************/
bool ofstream::open(const char* filename, ios::openmode mode)
/**************************************************************/
{
	if (mode==ios::out)
		_fileHandle = fopen(filename, "w");
	else if (mode == ios::app)
		_fileHandle = fopen(filename, "a");
	else if (mode == (ios::app | ios::out))
		_fileHandle = fopen(filename, "a");
	else  //Invalid open mode for an output stream
	{
		_fileHandle = NULL;
		//throw;
		return false;
	}

	return (is_open());
}

/**************************************************************/
bool ofstream::is_open()
/**************************************************************/
{
	if (_fileHandle!=NULL)
		return true;
	else
		return false;
}

/**************************************************************/
void ofstream::close()
/**************************************************************/
{
	if(_fileHandle!=NULL)
		fclose(_fileHandle);
}

}; /*end namespace ppcStreams*/