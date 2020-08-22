#include <ifstream>
#include <stdio.h>
#include <stdarg.h>

namespace std
{
/**************************************************************/
ifstream::ifstream()
/**************************************************************/
{
	_fileHandle = NULL;
}

/**************************************************************/
ifstream::ifstream(const char* filename)
/**************************************************************/
{
	open(filename);
}

/**************************************************************/
ifstream::~ifstream()
/**************************************************************/
{
}

/**************************************************************/
int ifstream::InFunction(const char* format, ...)
/**************************************************************/
{
	va_list argList;
	va_start(argList, format);
	return(fscanf(_fileHandle, format, va_arg(argList, double))); //Sloppy, stack manip problems
}

/**************************************************************/
int ifstream::read(char* buffer, int size)
/**************************************************************/
{
	return fread(buffer, size, 1, _fileHandle);
}

/**************************************************************/
bool ifstream::operator==(bool val)
/**************************************************************/
{
	return(is_open()==val);
}

/**************************************************************/
ifstream::operator void*()
/**************************************************************/
{
	if (is_open())
		return ((void*)this);
	else
		return NULL;
}

/**************************************************************/
bool ifstream::eof()
/**************************************************************/
{
	if(feof(_fileHandle))
	{
		ios::setstate(ios::eofbit);
		return true;
	}
	else
		return false;
}

/**************************************************************/
bool ifstream::open(const char* filename, ios::openmode mode)
/**************************************************************/
{
	if (mode==ios::in)
		_fileHandle = fopen(filename, "r");
	else //Invalid open mode
	{
		_fileHandle = NULL;
		//throw;
		return false;
	}
	return (is_open());
}

/**************************************************************/
bool ifstream::is_open()
/**************************************************************/
{
	if (_fileHandle!=NULL)
		return true;
	else
		return false;
}

/**************************************************************/
void ifstream::close()
/**************************************************************/
{
	if(_fileHandle!=NULL)
		fclose(_fileHandle);
}

int ifstream::tellg()
{
	return ftell(_fileHandle);
}

void ifstream::seekg(int pos, int whence)
{
	switch (whence)
	{
	case beg:
		fseek(_fileHandle, pos, SEEK_SET);
		break;
	case cur:
		fseek(_fileHandle, pos, SEEK_CUR);
		break;
	case end:
		fseek(_fileHandle, pos, SEEK_END);
		break;
	}
}

}; /*end namespace ppcStreams*/
