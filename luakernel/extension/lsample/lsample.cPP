#include "lsample.h"

extern "C" int lua_sum(lua_State* L)
{
	const int a = luaL_checkinteger(L, 1);
	const int b = luaL_checkinteger(L, 2);

	int sum = a + b;

	char buf[256];
	itoa(sum, 10, buf);
	lua_pushstring(L, buf);
	return 1;
}
#define luaL_register(L,name,reg) lua_newtable(L);luaL_setfuncs(L,reg,0)

static const luaL_Reg samplelib[] = {
	{"sum",         lua_sum         },
	{NULL, NULL}
};

extern "C" __declspec(dllexport) int luaopen_lsample(lua_State * L) 
{ 
	luaL_register(L, "lsample", samplelib);
	return 1; 
}
