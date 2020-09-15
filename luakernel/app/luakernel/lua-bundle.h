struct module
{
	char* name;
} lua_bundle[] =
{
	"font.lua",
	//"bit32.lua",
};

struct memory_module
{
	char* name;
	unsigned char* buf;
	unsigned int len;
} lua_memory_bundle[] =
{
	"null.lua",
};
