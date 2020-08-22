#pragma once
#include "windef.h"

class SkyQueue
{
public:
	SkyQueue();
	~SkyQueue();

	void Initialize(void* pvQueueBuffer, int iMaxDataCount, int iDataSize);
	bool IsQueueFull();
	bool IsQueueEmpty();
	bool PutQueue(const void* pvData);
	bool GetQueue(void* pvData);

private:
	int m_iDataSize;
	int m_iMaxDataCount;

	// 큐 버퍼의 포인터와 삽입/제거 인덱스
	void* m_pvQueueArray;
	int m_iPutIndex;
	int m_iGetIndex;

	// 큐에 수행된 마지막 명령이 삽입인지를 저장
	bool m_bLastOperationPut;
	

	void* m_pQueueBuffer;
};

