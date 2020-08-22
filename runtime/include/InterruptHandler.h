#pragma once

enum class InterruptStatus
{
	kUnhandledInterrupt,
	kHandledInterrupt,
	kReschedule
};

const int kMaxInterrupts = 32;

/// 하드웨어 인터럽트를 다루는 모든 클래스의 기본 클래스
class InterruptHandler
{
public:
	InterruptHandler(int vector, char* deviceName);
	virtual ~InterruptHandler();

	//인터럽트가 발생할때 호출되는 메소드. 파생 클래스는 이 메소드를 오버라이드에서 메소드를 구현해야 한다.
	// 반환값 : InterruptStatus, 인터럽트 체인 호출을 위해 사용
	// - kUnhandledInterrupt 
	// 해당 디바이스가 이 인터럽트를 발생시키지 않았다. 이 인터럽트를 다룰 수 있도록 다음 인터럽트 핸들러를 실행하라.
	// - kHandledInerrupt 
	// 이 드라이버가 인터럽트를 처리했다. 다른 인터럽트 핸들러는 이제 호출할 필요가 없다.
	// - kReschedule 
	// 이 드라이버가 인터럽트를 처리했다. 그리고 이 인터럽트는 하나 이상의 스레드를 실행상태로 변경했다.
	// 스레드들은 반응성이 높아야 하므로 인터럽트가 끝난후에 태스크를 스케쥴링한다.
	virtual InterruptStatus HandleInterrupt(void* arg = nullptr);

	int GetVectorNum() { return m_vectorNum; }

	InterruptHandler* m_pNext;
	int m_vectorNum;
	bool m_Active;
	char m_deviceName[32];
};