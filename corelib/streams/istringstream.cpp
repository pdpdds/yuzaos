#include <istringstream>

namespace std
{

/**************************************************************/
istringstream::istringstream(ios::openmode mode)
/**************************************************************/
{
	str("");
}

/**************************************************************/
istringstream::istringstream(string input, ios::openmode mode)
/**************************************************************/
{
	str(input);
}

/**************************************************************/
istringstream::~istringstream()
/**************************************************************/
{
}

/**************************************************************/
int istringstream::InFunction(const char* format, ...)
/**************************************************************/
{
	va_list argList;
	va_start(argList, format);
	char* pos = (char*)(_input.c_str() + _currentPosition);
	sscanf(pos, format, va_arg(argList, double));

	int bytesRead = 0;
	while (true)
	{
		bytesRead++;
		if ((pos[bytesRead]==' ')||(pos[bytesRead]=='\0')||(pos[bytesRead]==char(32)))
		{
			break;
		}
	}

	_currentPosition += bytesRead;

	//Quick hack here to fix to adjust current position if only single char comes in, needed for getline

	for (int c=0; c<strlen(format); c++)
	{
		if (format[c]=='%'&&format[c+1]=='c')
		{
			_currentPosition-=bytesRead;
			_currentPosition++;
			break;
		}
	} /*end quick hack*/

	return _currentPosition;
}

/**************************************************************/
string istringstream::str()
/**************************************************************/
{
	return(_input);
}

/**************************************************************/
void istringstream::str(string val)
/**************************************************************/
{
	_currentPosition = 0;
	_input = val;
}

}; /*end namespace ppcStreams */
