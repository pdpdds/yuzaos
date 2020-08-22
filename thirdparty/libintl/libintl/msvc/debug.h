
#ifdef _DEBUG

#ifndef _DEBUG_H_
#define _DEBUG_H_

#undef _malloca
#undef getcwd
#undef alloca

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#define alloca(x)     _malloca_dbg(x, _NORMAL_BLOCK, __FILE__, __LINE__)
#define _alloca(x)    _malloca_dbg(x, _NORMAL_BLOCK, __FILE__, __LINE__)

#endif

#endif

