#include <stdio.h>
#include "lua.hpp"
#include "lua-bundle.h"
#include <memory.h>
#include <string.h>
#include "luatinker.h"
#include <systemcall_impl.h>
#include <minwindef.h>
#include <lsample.h>

extern "C" int luaopen_lsqlite3(lua_State * L);
static uint32 keyboard_scancode_queue[8] = { 0 };
static uint32 keyboard_scancode_queue_len = 0;
static DWORD g_key_mutex = 0;

int g_display_width = 0;
int g_display_height = 0;
int g_display_bpp = 0;

uint8 *fbmem = nullptr;
uint8 *display_buffer = nullptr;
uint32 display_buffer_len = 0;
#define arraylen(array) (sizeof(array) / sizeof(array[0]))

lua_State* L = 0;

int lua_fopen(const char* pathname, int flags, int mode)
{
	return luatinker::call<int>(L, "open", flags, mode);
}

size_t lua_fwrite(int fd, const char* ptr)
{
	return luatinker::call<int>(L, "write", fd, ptr);
}

char* lua_fread(int fd, int count)
{
	return luatinker::call<char*>(L, "read", fd, count);
}

size_t lua_fseek(int fd, long int offset, int whence)
{
	return luatinker::call<int>(L, "lseek", offset, whence);
}

int clear_screen(lua_State *L)
{
	memset(display_buffer, 0, display_buffer_len);
	return 0;
}

static int putpixel(lua_State *l)
{
	uint32 x = lua_tonumber(l, 1);
	uint32 y = lua_tonumber(l, 2);
	uint32 r = lua_tonumber(l, 3);
	uint32 g = lua_tonumber(l, 4);
	uint32 b = lua_tonumber(l, 5);
	lua_pop(l, 5);
	const uint32 bytes_per_pixel = g_display_bpp;

	uint8 *ptr = &display_buffer[(y * g_display_width * bytes_per_pixel) + (x * bytes_per_pixel)];

	const uint8 *display_buffer_end = &display_buffer[g_display_height * g_display_width * bytes_per_pixel -1];

	if (ptr < display_buffer_end)
	{
		ptr[0] = b;
		ptr[1] = g;
		ptr[2] = r;
		ptr[3] = 0;
	}
	else
	{
		Syscall_Panic("putpixel");
	}

	return 0;
}

static int file_test(lua_State* l)
{
	int fd = lua_fopen("test", 64, 0);
	size_t written = lua_fwrite(fd, "test sentence");
	lua_fseek(fd, 0, SEEK_SET);
	
	char* buf = lua_fread(fd, strlen("test sentence"));
	return 0;
}

int kMemCpy(void* pvDestination, const void* pvSource, int iSize)
{
	int i;
	int iRemainByteStartOffset;

	// 8 바이트씩 먼저 복사
	for (i = 0; i < (iSize / 8); i++)
	{
		((QWORD*)pvDestination)[i] = ((QWORD*)pvSource)[i];
	}

	// 8 바이트씩 채우고 남은 부분을 마무리
	iRemainByteStartOffset = i * 8;
	for (i = 0; i < (iSize % 8); i++)
	{
		((char*)pvDestination)[iRemainByteStartOffset] =
			((char*)pvSource)[iRemainByteStartOffset];
		iRemainByteStartOffset++;
	}
	return iSize;
}

static int swap_buffers(lua_State *l)
{
	kMemCpy(fbmem, display_buffer, display_buffer_len);
	clear_screen(l);
	return 0;
}

// http://lua-users.org/lists/lua-l/2003-12/msg00301.html
// http://lua-users.org/lists/lua-l/2002-12/msg00171.html
// http://lua-users.org/lists/lua-l/2011-06/msg00426.html
// http://lua-users.org/lists/lua-l/2010-03/msg00679.html

static void lua_hook(lua_State *l, lua_Debug *ar)
{
	lua_yield(l, 0);
}

static int lua_setmaskhook(lua_State *l)
{
	lua_State *t = lua_tothread(l, 1);
	int maskcount = lua_tointeger(l, 2);
	lua_pop(l, 2);
	if (t)
	{
		lua_sethook(t, lua_hook, LUA_MASKCOUNT, maskcount);
	}
	return 0;
}

static int lua_get_timer_ticks(lua_State *l)
{
	unsigned int tickCount = Syscall_GetTickCount();
	lua_pushinteger(l, tickCount);
	return 1;
}

static int lua_sleep(lua_State *l)
{
	Syscall_Sleep(0);
	return 0;
}

int lua_get_keyboard_interrupt(lua_State *l)
{
	// disable interrupts	
	Syscall_LockMutex(g_key_mutex);

	
	lua_createtable(l, keyboard_scancode_queue_len, 0);
	for (int i = 0; i < keyboard_scancode_queue_len; ++i)
	{
		lua_pushinteger(l, keyboard_scancode_queue[i]);
		lua_rawseti(l, -2, (i + 1));
	}
	keyboard_scancode_queue_len = 0;
	Syscall_UnlockMutex(g_key_mutex);

	return 1;
}

static int lua_hlt(lua_State *l)
{
	//__asm hlt;
	return 0;
}

const char *errstr = NULL;

static int lua_loader(lua_State *l)
{
	size_t len;
	const char *modname = lua_tolstring(l, -1, &len);
	struct module *mod = NULL;
	for (int i = 0; i < arraylen(lua_bundle); ++i)
	{
		if (memcmp(modname, lua_bundle[i].name, len) == 0)
		{
			mod = &lua_bundle[i];
		}
	}
	if (!mod)
	{
		lua_pushnil(l);
		return 1;
	}
	//if (luaL_loadbuffer(l, (const char*)mod->buf, mod->len, (const char*)mod->name) != LUA_OK)
	if (luaL_loadfile(l, (const char*)mod->name) != LUA_OK)
	{
		errstr = lua_tostring(l, 1);
		//~ puts("luaL_loadstring: error");
		//trap();
	}
	int err = lua_pcall(l, 0, LUA_MULTRET, 0);
	if (err != LUA_OK)
	{
		errstr = lua_tostring(l, 1);
		//~ puts("lua_pcall: error");
		//trap();
	}
	if (!lua_istable(l, -1))
	{
		printf("not a table");
	}
	return 1;
}

static void *l_alloc(void *ud, void *ptr, size_t osize, size_t nsize) 
{
	//(void)ud; (void)osize;  /* not used */

	if (nsize == 0) {

		if(ptr)
			free(ptr);
		return NULL;
	}
	else
		return realloc(ptr, nsize);
}

bool lua_main(uint8* frameBuffer, int width, int height, int bpp)
{	
	g_display_width = width;
	g_display_height = height;
	g_display_bpp = bpp;

	display_buffer_len = (width * height * bpp);
	display_buffer = (uint8*)malloc(display_buffer_len);

	//L = lua_newstate(l_alloc, NULL);
	L = luaL_newstate();

	g_key_mutex = Syscall_CreateMutex("key_mutex");
	if (!L)
	{
		fputs("lua_newstate: error", stdin);
		return false;
	}

	luaopen_base(L);
	luaL_openlibs(L);
	//lua_gc(L, LUA_GCGEN, 0, 0);  /* GC in generational mode */

	fbmem = frameBuffer;
	
	lua_pushnumber(L, g_display_width);
	lua_setglobal(L, "DISPLAY_WIDTH");
	lua_pushnumber(L, g_display_height);
	lua_setglobal(L, "DISPLAY_HEIGHT");
	
	luatinker::def(L, "clear_screen", clear_screen);
	luatinker::def(L, "putpixel", putpixel);
	luatinker::def(L, "swap_buffers", swap_buffers);
	luatinker::def(L, "file_test", file_test);
	luatinker::def(L, "sleep", lua_sleep);
	luatinker::def(L, "setmaskhook", lua_setmaskhook);
	luatinker::def(L, "loader", lua_loader);
	luatinker::def(L, "get_timer_ticks", lua_get_timer_ticks);	
	luatinker::def(L, "get_keyboard_interrupt", lua_get_keyboard_interrupt);
	luatinker::def(L, "hlt", lua_hlt);
	luaL_requiref(L, "lsqlite3", luaopen_lsqlite3, 0);
	luaL_requiref(L, "lsample", luaopen_lsample, 0);
	lua_pop(L, 1);

	if (luaL_loadfile(L, "luakernel.lua") != LUA_OK)
	{		
		errstr = lua_tostring(L, 1);

		if(errstr)
			printf("error %str\n", errstr);
		Syscall_Panic("Lua Script Load Fail!!");
	}

	//Print(0, "Welcome to Lua Kernel!!\n");
	//Print(0, "Lua Version is 5.40\n");

	int err = lua_pcall(L, 0, LUA_MULTRET, 0);
	if (err != LUA_OK)
	{
		printf("errorcode %d\n", err);
		Syscall_Panic("Lua Script Load Fail!!");
		return false;
	}
	printf("success %d\n", err);
	Syscall_Panic("Lua Script Load Fail!!");
	return true;
}

bool HandleInterrupt(unsigned char scanCode)
{
	
	//u32 scancode = inb(0x60);
	Syscall_LockMutex(g_key_mutex);
	if (keyboard_scancode_queue_len < arraylen(keyboard_scancode_queue))
	{
		keyboard_scancode_queue[keyboard_scancode_queue_len] = scanCode;
		keyboard_scancode_queue_len += 1;
	}
	Syscall_UnlockMutex(g_key_mutex);

	return true;
}

int HandleTerminalWrite(char* msg, int len)
{
	if (!L || len == 0)
		return false;
	
	int fd = 1;
	
	return lua_fwrite(fd, msg);
}

