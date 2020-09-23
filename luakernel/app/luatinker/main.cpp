#include "test.h"
#include <windef.h>
#include <stdio.h>
#include "GUIConsoleFramework.h"

int main_impl(int argc, char** argv)
{
	lua_State* L;

	L = luaL_newstate();

	luaopen_base(L);

	//test1(L);
	//test2(L);
	//test3(L);
	//test4(L);
	//test5(L);
	test6(L);

	lua_close(L);


	return 0;
}

int main(int argc, char** argv)
{
	GUIConsoleFramework framework;
	return framework.Run(argc, argv, main_impl);
}