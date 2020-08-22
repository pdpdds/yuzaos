#pragma once
#include "Resource.h"
#include <ds/queue.h>

/// Implementation of a counting semaphore
class Semaphore : public Resource 
{
public:
	Semaphore(const char name[], int initialCount = 1);

	void Release(int releaseCount = 1, bool reschedule = true);
	void Reset(int releaseCount = 0);

	int GetCount() {return fCount;}
protected:
	virtual void ThreadWoken();
private:
	volatile int fCount;
};

