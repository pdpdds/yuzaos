#pragma once
#include <windef.h>
#include <minwinbase.h>

#ifdef __cplusplus
extern "C"
{
#endif

	struct dirent
	{
		char d_name[MAX_PATH];
		DWORD dwAttribute;
		unsigned int fsize;
	};

	typedef ptrdiff_t handle_type; /* C99's intptr_t not sufficiently portable */

	struct DIR
	{
		handle_type         handle; /* -1 for failed rewind */
		WIN32_FIND_DATA     info;
		struct dirent       result; /* d_name null iff first time */
		char* name;  /* null-terminated char string */
	};

#ifdef __cplusplus
}
#endif
