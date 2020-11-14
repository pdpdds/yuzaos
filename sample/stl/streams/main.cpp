#include <stdio.h>
#include <iostream>
#include <fstream>

void* operator new[](size_t size, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
	return new uint8_t[size];
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return new uint8_t[size];
}

using namespace eastl;

int main(int argc, char** argv)
{
	ofstream file;
	file.open("example.txt");
	file << "Yuza OS Test!!\n";
	file.close();
	return 0;
}