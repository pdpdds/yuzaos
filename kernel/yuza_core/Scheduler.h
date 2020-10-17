#pragma once
#include "Timer.h"
#include "ktypes.h"

const int kMaxPriority = 31;

class Thread;

//스케쥴러 클래스. 다음에 실행될 스레드를 선택한다.
class _Scheduler : public Timer
{
public:
	_Scheduler();

	//실행될 다음 스레드를 선택하고 SwichTo 메소드를 호출해서 해당 스레드 컨텍스트로 전환한다.
	void Reschedule();

	// 실행 준비가 된 스레드를 레디 큐에 넣는다.
	// kThreadReady
	void EnqueueReadyThread(Thread* pThread);

private:
	Thread* PickNextThread();
    virtual InterruptStatus HandleTimeout() override;

	int fReadyThreadCount;
	int fHighestReadyThread;
	_Queue fReadyQueue[kMaxPriority + 1];
};

extern _Scheduler gScheduler;