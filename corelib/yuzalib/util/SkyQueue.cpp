#include <SkyQueue.h>
#include <memory.h>

SkyQueue::SkyQueue()
{
}


SkyQueue::~SkyQueue()
{
}

void SkyQueue::Initialize(void* pvQueueBuffer, int iMaxDataCount, int iDataSize)
{
	m_iMaxDataCount = iMaxDataCount;
	m_iDataSize = iDataSize;
	m_pvQueueArray = pvQueueBuffer;

	m_iPutIndex = 0;
	m_iGetIndex = 0;
	m_bLastOperationPut = false;
}

bool SkyQueue::IsQueueFull()
{
	if ((m_iGetIndex == m_iPutIndex) &&
		(m_bLastOperationPut == true))
	{
		return true;
	}
	return false;
}

bool SkyQueue::IsQueueEmpty()
{
	if ((m_iGetIndex == m_iPutIndex) &&(m_bLastOperationPut == false))
	{
		return true;
	}

	return false;
}

bool SkyQueue::PutQueue(const void* pvData)
{	
	if (IsQueueFull() == true)
	{
		return false;
	}

	memcpy((char*)m_pvQueueArray + (m_iDataSize * m_iPutIndex), pvData, m_iDataSize);

	m_iPutIndex = (m_iPutIndex + 1) % m_iMaxDataCount;
	m_bLastOperationPut = true;
	return true;
}


bool  SkyQueue::GetQueue(void* pvData)
{	
	if (IsQueueEmpty() == true)
	{
		return false;
	}

	memcpy(pvData, (char*)m_pvQueueArray + (m_iDataSize * m_iGetIndex), m_iDataSize);

	m_iGetIndex = (m_iGetIndex + 1) % m_iMaxDataCount;
	m_bLastOperationPut = false;
	return true;
}

