#include <stdio.h>
#include <EASTL/array.h>
#include <EASTL/algorithm.h>
#include <EASTL/sort.h>
#include <functional.hpp>

void* operator new[](size_t size, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
	return new uint8_t[size];
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return new uint8_t[size];
}

int add(int first, int second)
{
	return first + second;
}

using namespace eastl;

int main(int argc, char** argv)
{
	using namespace std::placeholders;
	auto add_func = std::_bind::bind(&add, _1, _2);
	add_func(100, 101);
}

