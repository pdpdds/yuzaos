#include "ktypes.h"
#include "Interrupt.h"
#include "InterruptManager.h"
#include <string>
#include "AddressSpace.h"
#include "Debugger.h"

#include <stringdef.h>
#include "SystemCall.h"
#include "Thread.h"
#include "ThreadContext.h"
#include "Scheduler.h"
#include "memory_layout.h"
#include "intrinsic.h"
#include <StackTracer.h>

const int kUserCs = 0x1b;

extern "C" {	
	void BadTrap();
	void HandleTrap(InterruptFrame);
};

void InterruptFrame::Print() const
{
	const char kFlagLetters[] = "cxpxaxzstipollnxrv";
	kprintf("eax %x       edi %x\n", eax, edi);
	kprintf("ebx %x       esi %x\n", ebx, esi);
	kprintf("ecx %x       ebp %x\n", ecx, ebp);
	kprintf("edx %x       ", edx);
	if ((cs & 3) == 0)
		kprintf("esp %x\n", esp);
	else
		kprintf("esp %04x:%x\n", user_ss, user_esp);
	
	kprintf("eip %x:%x  ", cs, eip);
	kprintf("eflags ");
	for (int i = 17; i >= 0; i--)
		kprintf("%c", (flags & (1 << i)) ? kFlagLetters[i] : kFlagLetters[i] +
			('A' - 'a'));
	kprintf("\ntrap %x      error code %x\n", vector, errorCode);

	StackTracer::GetInstance()->TraceStackWithSymbol();
}

extern unsigned int g_tickCount;
void HandleTrap(InterruptFrame iframe)
{
	switch (iframe.vector) 
	{
	case kDebugTrap: 
	{
		unsigned int status = GetDR6();
		if (status & 7) {
			kprintf("Breakpoint\n");
			iframe.Print();
		}
		else if ((status & (1 << 14)) == 0) {
			kprintf("Unknown debug Trap\n");
			iframe.Print();
		}

		Debugger::GetInstance()->DebugKernel();
		break;
	}

	case kDeviceNotAvailable:
		ThreadContext::SwitchFp();
		break;

	case kPageFault: 
	{
		// It is important to get the fault address (from cr2) before re-enabling
		// interrupts because it is not saved across task switches.
		unsigned int va = GetFaultAddress();
		EnableInterrupts();

#if DEBUG_SHIT
		if (iframe.errorCode & kPageFaultReserved)
			panic("Bad PTE entry (kernel fucked up)");
#endif

		AddressSpace *space = 0;

		if (va < kUserBase && va > kUserTop)
		{
			space = AddressSpace::GetKernelAddressSpace();
		}
		else
		{
			space = AddressSpace::GetCurrentAddressSpace();
		}
		//AddressSpace *space = va >= kKernelBase
			//? AddressSpace::GetKernelAddressSpace()
			//: AddressSpace::GetCurrentAddressSpace();
		if (space->HandleFault(va, iframe.errorCode & kPageFaultWrite, iframe.errorCode & kPageFaultUser) < 0) 
		{
			// Invalid page fault.  If there is a fault handler (used by CopyUser
			// functions), jump to it.
			if (Thread::GetRunningThread()->GetFaultHandler()) 
			{
				kprintf("invoking fault handler\n");
				iframe.eip = Thread::GetRunningThread()->GetFaultHandler();
			}
			else 
			{
				kprintf("unhandled page fault.\n");
				kprintf("Thread %s in %s mode attempted to %s %s address %x\n",
					Thread::GetRunningThread()->GetName(),
					(iframe.errorCode & kPageFaultUser) ? "user" : "supervisor",
					(iframe.errorCode & kPageFaultWrite) ? "write" : "read",
					(iframe.errorCode & kPageFaultProtection) ? "protected" : "unmapped", va);
				iframe.Print();
				Debugger::GetInstance()->DebugKernel();
			}
		}

		break;
	}

	case kSystemCall: {
		/*EnableInterrupts();
		const SystemCallInfo &info = systemCallTable[iframe.eax & 0xff];
		iframe.eax = InvokeSystemCall(&info.hook, reinterpret_cast<int*>(iframe.user_esp) + 1,
			info.parameterSize);
		*/
		break;
	}

	case 32: case 33: case 34: case 35: case 36: case 37: case 38: case 39:
	case 40: case 41: case 42: case 43: case 44: case 45: case 46: case 47: 
	{
		InterruptStatus result = InterruptManager::Dispatch(iframe.vector - 32);

		// Note: this assumes level triggered interrupts.  It works
		// with edge triggered ones, but may lose interrupts if the
		// ISR is slow.
		if (iframe.vector - 32 > 7)
			OutPortByte(0xa0, 0x20);	// EOI to pic 2

		OutPortByte(0x20, 0x20);	// EOI to pic 1

		if (result == InterruptStatus::kReschedule)
			gScheduler.Reschedule();
		else if (result == InterruptStatus::kUnhandledInterrupt) {
			iframe.Print();
			kPanic("Unhandled Interrupt %d\n", iframe.vector);
		}

		break;
	}

	default:
		printf("Unknown trap %d occured.\n", iframe.vector);
		iframe.Print();
		Debugger::GetInstance()->DebugKernel();
	}

	// Dispatch APC if one is pending.  This is only done before switching
	// back to user mode.	
	if (iframe.cs == kUserCs) {
		APC *apc = Thread::GetRunningThread()->DequeueAPC();
		if (apc) {
			APC temp = *apc;
			delete apc;
			if (temp.fIsKernel)
				(*temp.fCallback)(temp.fData);
			else
				kPanic("User APCs not implemented\n");
		}
	}
}
