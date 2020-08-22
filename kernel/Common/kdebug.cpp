#include <stdio.h>
#include <string.h>
#include <systemcall_impl.h>
#include <minwindef.h>
#include <SkyConsole.h>

static char* sickpc = " (>_<) YUZA OS Error!!\n\n";

char* szDisclame = "We apologize, SkyOS has encountered a problem and has been shut down\n\
to prevent damage to your computer. Any unsaved work might be lost.\n\
We are sorry for the inconvenience this might have caused.\n\n\
Please report the following information and restart your computer.\n\
The system has been halted.\n\n";

extern void SampleFillRect(ULONG* lfb0, int x, int y, int w, int h, int col);

void kHaltSystem(const char* errMsg)
{
	if (Syscall_IsGraphicMode())
	{
		/*ULONG* bufferAddess = (ULONG*)g_bootParams.framebuffer_addr;
		SampleFillRect(bufferAddess, 1004, 0, 20, 20, 0xFF00FF00);
		kDebugPrint(errMsg);*/
	}
	else
	{
		SkyConsole::MoveCursor(0, 0);
		SkyConsole::SetColor(ConsoleColor::White, ConsoleColor::Blue, false);
		SkyConsole::Clear();
		SkyConsole::Print(sickpc);

		SkyConsole::Print("*** STOP: %s", errMsg);
	}
	for (;;);
}

extern "C" void kPanic(const char* fmt, ...)
{
	char buf[1024];

	va_list arglist;
	va_start(arglist, fmt);
	vsnprintf(buf, 1024, fmt, arglist);

	kHaltSystem(buf);
}

void __SKY_ASSERT(const char* expr_str, bool expr, const char* file, int line, const char* msg)
{
	if (!expr)
	{
		kPanic("Assert failed: %s Expected: %s %s %d\n", msg, expr_str, file, line);
	}
}