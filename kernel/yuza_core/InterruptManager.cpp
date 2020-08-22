#include "InterruptManager.h"
#include "systemcall_impl.h"
#include "intrinsic.h"
#include <stdio.h>
#include <SystemAPI.h>

InterruptHandler* InterruptManager::s_handlers[kMaxInterrupts] = { 0, };

void InterruptManager::ObserveInterrupt(int vectorNum, InterruptHandler* pHandler)
{
	kprintf("ObserveInterrupt %x\n", vectorNum);
	
	if (vectorNum < 0 || vectorNum > kMaxInterrupts)
		Syscall_Panic("Wrong Interrupt vectorNum\n");

	int st = DisableInterrupts();
	
//현재 이 인터럽트에 관심을 가진 핸들러가 하나도 없었다면
//해당 인터럽트가 발생하도록 한다..
	if (s_handlers[vectorNum] == 0)
		EnableIrq(vectorNum);

//핸들러 추가시 이 핸들러는 핸들러 리스트에서 선두에 온다.
	pHandler->m_pNext = s_handlers[vectorNum];
	s_handlers[vectorNum] = pHandler;
	pHandler->m_vectorNum = vectorNum;

	RestoreInterrupts(st);

	
}

void InterruptManager::IgnoreInterrupts(InterruptHandler* pHandler)
{
	if(pHandler == nullptr)
		Syscall_Panic("Interrupt Handler is nullptr");

	int vectorNum = pHandler->GetVectorNum();

	if (vectorNum < 0 || vectorNum >= kMaxInterrupts)
		Syscall_Panic("Attempt to handler that isn't registered");

	int st = DisableInterrupts();

	if (s_handlers[vectorNum] == 0)
		Syscall_Panic("Attempt to remove interrupt handler that is not installed");

	if (s_handlers[vectorNum] == pHandler)
		s_handlers[vectorNum] = pHandler->m_pNext;
	else
	{
		for (InterruptHandler* handler = s_handlers[vectorNum];; handler = handler->m_pNext)
		{
			if (handler->m_pNext == 0)
				Syscall_Panic("Interrupt handler is registered, but is not in list");

			if (handler->m_pNext == pHandler) 
			{
				handler->m_pNext = handler->m_pNext->m_pNext;
				break;
			}
		}
	}

//인트럽트 핸들러는 핸들러 리스트에서 제거되었다. 이제 제거된 핸들러가 관심을 가졌던
//인터럽트에 대해 관심을 가지는 다른 핸들러가 하나도 존재하지 않는다면
//이제 해당 인터럽트가 발생하지 않게 한다.
	if (s_handlers[vectorNum] == 0)
		DisableIrq(vectorNum);

	pHandler->m_vectorNum = -1;
	RestoreInterrupts(st);
}

InterruptStatus InterruptManager::Dispatch(int vectorNum)
{
	InterruptStatus result = InterruptStatus::kUnhandledInterrupt;
	for (InterruptHandler* handler = s_handlers[vectorNum]; handler != 0 ; handler = handler->m_pNext)
	{
		if (handler->m_Active)
			Syscall_Panic("Interrupt handler called reentrantly");

		handler->m_Active = true;
//  이 인터럽트에 관심이 있는 모든 핸들러의 HandleInterrupt를 호출한다.
		result = handler->HandleInterrupt();
		handler->m_Active = false;

//여러 핸들러가 이 인터럽트에 관심을 가지고 있다 하더라도 인터럽트는 하나의 핸들러에만 유효할 것이다.
//그래서 인터럽트 처리가 성공했다면 굳이 나머지 핸들러에 대해서 인터럽트 처리를 할 필요가 없다. 
		if (result != InterruptStatus::kUnhandledInterrupt)
			break;
	}

	if (result == InterruptStatus::kUnhandledInterrupt)
		kprintf("Unhandled Dispatch %x\n", vectorNum);

	return result;
}