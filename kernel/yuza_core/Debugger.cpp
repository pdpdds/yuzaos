#include "Debugger.h"
#include <string>
#include "cpu_asm.h"
#include <stringdef.h>
#include "interrupt.h"
#include <List.h>
#include "memory.h"
#include "intrinsic.h"
#include <systemcall_impl.h>

static void DoHelp(int, const char**);
static void DoMemoryDump(int argc, const char* argv[]);
static void DoUptime(int, const char* []);

const int kBaudRate = 115200;

const int kMaxDebugCommands = 64;
const int kCommandBufferLength = 128;
const int kMaxArgs = 16;
const int kCommandHistorySize = 10;
const int kBufferLength = 256;
static char tempBuffer[kBufferLength];
static int commandSlot;
static char commandHistory[kCommandHistorySize][kCommandBufferLength];
static bigtime_t bootTime;
static struct {
	const char* name;
	const char* description;
	DebugHook hook;
} debugCommands[kMaxDebugCommands];

Debugger* Debugger::m_pDebugger = 0;

Debugger::Debugger()
{

}

void Debugger::Bootstrap()
{
	bootTime = SystemTime();

	AddCommand("help", "List debugger commands", DoHelp);
	AddCommand("dm", "Show memory", DoMemoryDump);
	AddCommand("uptime", "Time the machine has been running", DoUptime);
}

void Debugger::DebugKernel()
{
	kprintf("\nOrangeOS Kernel Debugger\n");

	int fl = DisableInterrupts();
	char	commandBuffer[MAXPATH];

	for (;;) {
		kprintf("Debugger> ");

		if (ExecuteCommand(commandBuffer) == false)
			break;

		Syscall_GetCommandFromKeyboard(commandBuffer, MAXPATH - 2);

	}

	RestoreInterrupts(fl);
}

bool Debugger::ExecuteCommand(char* commandBuffer)
{
	int fl = DisableInterrupts();

	const char* argv[kMaxArgs];
	int argc = ParseArguments(commandBuffer, argv, kMaxArgs);
	if (argc == 0)
		return true;

	if (strcmp(argv[0], "continue") == 0 || strcmp(argv[0], "c") == 0)
		false;

	DebugHook hook = 0;
	for (int index = 0; index < kMaxDebugCommands; index++)
	{
		if (debugCommands[index].hook != 0 && strcmp(debugCommands[index].name, argv[0]) == 0)
		{
			hook = debugCommands[index].hook;
			break;
		}
	}

	kprintf("\n");
	if (hook)
		hook(argc, argv);
	else
		kprintf("syntax error\n");

	RestoreInterrupts(fl);

	return true;
}

void Debugger::AddCommand(const char name[], const char description[], DebugHook hook)
{
	for (int index = 0; index < kMaxDebugCommands; index++) {
		if (debugCommands[index].hook == 0) {
			debugCommands[index].name = name;
			debugCommands[index].description = description;
			debugCommands[index].hook = hook;
			break;
		}
	}
}

void Debugger::RemoveCommand(DebugHook hook)
{
	for (int index = 0; index < kMaxDebugCommands; index++) {
		if (debugCommands[index].hook == hook) {
			debugCommands[index].hook = 0;
			break;
		}
	}
}

void Debugger::BinDump(const char data[], int size)
{
	const int kColumnCount = 16;
	for (int rowStart = 0; rowStart < size; rowStart += kColumnCount) {
		kprintf("\n%p  ", data + rowStart);
		for (int column = 0; column < kColumnCount; column++)
			kprintf("%02x ", data[rowStart + column]);

		kprintf(" ");
		for (int column = 0; column < kColumnCount; column++) {
			char c = data[rowStart + column];
			if (c > 32)
				kprintf("%c", c);
			else
				kprintf(".");
		}
	}

	kprintf("\n");
}

int Debugger::ParseArguments(char inBuffer[], const char* arguments[], int kMaxArgs)
{
	int argumentCount = 0;
	enum {
		kScanArg,
		kScanSpace
	} state = kScanSpace;

	for (char* in = inBuffer; *in != 0 && argumentCount < kMaxArgs; in++) {
		switch (state) {
		case kScanArg:
			if (isspace(*in)) {
				state = kScanSpace;
				*in = '\0';
			}

			break;

		case kScanSpace:
			if (!isspace(*in)) {
				state = kScanArg;
				arguments[argumentCount++] = in;
			}

			break;
		}
	}

	return argumentCount;
}

void DoHelp(int, const char**)
{
	kprintf("\nCommands:\n");
	for (int index = 0; index < kMaxDebugCommands; index++)
		if (debugCommands[index].hook)
			kprintf("%s %s\n", debugCommands[index].name, debugCommands[index].description);
}

void DoUptime(int, const char* [])
{
	bigtime_t now = SystemTime() - bootTime;
	//bigtime_t milliseconds = now / 1000;
	bigtime_t milliseconds = now;
	bigtime_t seconds = milliseconds / 1000;
	bigtime_t minutes = seconds / 60;
	bigtime_t hours = minutes / 60;
	bigtime_t days = hours / 24;
	kprintf("System has been running for ");
	if (days > 0)
		kprintf("%Q days, ", days);

	if (hours > 0 || days > 0)
		kprintf("%Q hours, ", hours % 24);

	if (minutes > 0 || hours > 0 || days > 0)
		kprintf("%Q minutes, ", minutes % 60);

	kprintf("%Q.%Q seconds\n", seconds % 60, milliseconds % 1000);
}

static void DoMemoryDump(int argc, const char* argv[])
{
	if (argc != 3)
		kprintf("usage: %s <start> <size>\n", argv[0]);
	else
		Debugger::GetInstance()->BinDump(reinterpret_cast<const char*>(atoi(argv[1])), atoi(argv[2]));
}

