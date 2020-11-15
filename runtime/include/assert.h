#pragma once

#define assert(a) {if (!(a)) __asm hlt}	
#define ASSERT(a) assert(a)
