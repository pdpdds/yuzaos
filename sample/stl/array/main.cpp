#include <stdio.h>
#include <EASTL/array.h>
#include <EASTL/algorithm.h>
#include <EASTL/sort.h>
#include <iostream>

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
    array<int, 4> a = { 47, 23, 90, 1 };

    sort(a.begin(), a.end());

    for (auto i = a.begin(); i != a.end(); ++i)
    {
        cout << *i << endl;
    }

	return 0;
}

