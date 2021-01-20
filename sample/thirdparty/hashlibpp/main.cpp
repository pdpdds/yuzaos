#include <windef.h>
#include "stdio.h"
#include "string.h"
#include <memory.h>
#include <SystemCall_Impl.h>
#include <src\hashlibpp.h>
#include <string>

int main(int argc, char** argv)
{

	hashwrapper *myWrapper = new sha1wrapper();

	/*
	* create a hash from a string
	*/
	std::string hash1 = myWrapper->getHashFromString("Hello World");

	/*
	* create a hash based on a file
	*/
	std::string hash2 = myWrapper->getHashFromFile("README.TXT");

	delete myWrapper;

	return 0;
}

