#include <stdio.h>
#include <EASTL/algorithm.h>
#include <EASTL/string.h>

void* operator new[](size_t size, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
	return new uint8_t[size];
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return new uint8_t[size];
}

using namespace eastl;

eastl::string trimLeft(const eastl::string& s) {
    auto temp = s;
    temp.erase(eastl::begin(temp), eastl::find_if(eastl::begin(temp), eastl::end(temp),
        [](char c) {return !isspace(c);}));
    return temp;
}

eastl::string trimRight(const eastl::string& s) {
    auto temp = s;
    temp.erase(eastl::find_if(eastl::rbegin(temp), eastl::rend(temp),
        [](char c) {return !isspace(c); }).base(),
        eastl::end(temp));
    return temp;
}

eastl::string trim(const eastl::string& s) {
    return trimLeft(trimRight(s));
}


int main(void)
{
    eastl::string str = "  YUZA OS Trim Test!!     ";
    trim(str);

    printf("Trim Result[%s]\n", str.c_str());

    return 0;
}