#include <stdio.h>
#include <EASTL/tuple.h>

void* operator new[](size_t size, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
	return new uint8_t[size];
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return new uint8_t[size];
}

eastl::tuple<int, int, int, int> MakeSampleTuple(int a, int b)
{
	return eastl::make_tuple(a + b, a - b, a * b, a / b);
}

int main(void)
{
	int add, sub, div;
	eastl::tie(add, sub, eastl::ignore, div) = MakeSampleTuple(15, 18);

	printf("%d, %d, %d\n", add, sub, div);

	return 0;
}