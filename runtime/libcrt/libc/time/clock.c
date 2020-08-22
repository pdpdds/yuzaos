#include <orangeos.h>
#include <../include/SystemCall_Impl.h>

clock_t clock(void)
{
    static clock_t SysTick = 0;
	static bool init = 0;

	if (init == false)
	{
		SysTick = Syscall_GetTickCount();
		init = true;
	}

	return Syscall_GetTickCount()- SysTick;
}
