#include "InterruptHandler.h"
#include <systemcall_impl.h>
#include <stringdef.h>

InterruptHandler::InterruptHandler(int vectorNum, char* deviceName)
	: m_vectorNum(vectorNum),
	  m_pNext(nullptr),
	  m_Active(false)
{
	if(deviceName  != 0)
		strcpy(m_deviceName, deviceName);
}

InterruptHandler::~InterruptHandler()
{
	if (m_Active)
		Syscall_Panic("Attempt to delete active interrupt handler!");

	if (m_vectorNum != -1)
		Syscall_Panic("Attept to delete registered interrupt handler!\n");
}

InterruptStatus InterruptHandler::HandleInterrupt(void* arg)
{
	return InterruptStatus::kUnhandledInterrupt;
}

