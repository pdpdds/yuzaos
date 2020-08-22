#pragma once
#include <InterruptHandler.h>

class InterruptManager
{
public:
	/// Begin watching an interrupt with the given interrupt vector.  If other InterruptHandlers
	/// are already watching this interrupt, this handler will be chained with them.
	/// @bug The behavior is undefined if you try to watch the same interrupt twice.  Should assert.
	static void ObserveInterrupt(int vector, InterruptHandler* pHandler);
	
	// 특정 인터럽트를 무시한다.
	static void IgnoreInterrupts(InterruptHandler* pHandler);

	/// 인터럽트 발생시 디바이스 독립적인 인터럽트 핸들러 호출
	static InterruptStatus Dispatch(int vectorNum);

private:
	static InterruptHandler* s_handlers[kMaxInterrupts];
};