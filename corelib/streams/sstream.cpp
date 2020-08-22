#include <sstream>
#include <assert.h>

namespace std
{

/**************************************************************/
stringstream::stringstream(istream::openmode mode)
/**************************************************************/
{
	_currentPosition = 0;
}

/**************************************************************/
stringstream::stringstream(string inout, istream::openmode mode)
/**************************************************************/
{
	_currentPosition = 0;
	str(inout);
}

/**************************************************************/
stringstream::~stringstream()
/**************************************************************/
{
}

/**************************************************************/
int stringstream::InFunction(const char* format, ...)
/**************************************************************/
{
	va_list argList;
	va_start(argList, format);
	_currentPosition = _currentPosition + _input.InFunction(format, va_arg(argList, double));
	return (_currentPosition);
}

/**************************************************************/
int stringstream::OutFunction(const char* format, ...)
/**************************************************************/
{
	va_list argList;
	va_start(argList, format);
	int retValue = 0;
	retValue = _output.OutFunction(format, va_arg(argList, double));

	//Update input string stream's string
	_input.str(_output.str());

	//As current position in input got reset, set it to the correct value
	_input._currentPosition = _currentPosition; //TODO: Fix this violation of encapsulation

	return(retValue);
}

/**************************************************************/
string stringstream::str()
/**************************************************************/
{
	assert(_input.str()==_output.str());
	return(_input.str()); //return either input or output, both should be synced
}

/**************************************************************/
void stringstream::str(string val)
/**************************************************************/
{
	_output.str(val);
	_input.str(val);
}

}; /*end namespace ppcStreams*/
