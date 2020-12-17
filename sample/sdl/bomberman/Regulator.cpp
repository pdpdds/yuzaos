#include "Regulator.h"
#include "Utils.h"

#ifdef _WIN32
#include <Windows.h>
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")
#endif

Regulator::Regulator(double NumUpdatesPerSecondRqd)
{
	m_dwNextUpdateTime = (unsigned int)(timeGetTime() + RandFloat() * 1000);

	if (NumUpdatesPerSecondRqd > 0)
	{
		m_dUpdatePeriod = 1000.0 / NumUpdatesPerSecondRqd;
	}

	else if (isEqual(0.0, NumUpdatesPerSecondRqd))
	{
		m_dUpdatePeriod = 0.0;
	}

	else if (NumUpdatesPerSecondRqd < 0)
	{
		m_dUpdatePeriod = -1;
	}
}


//returns true if the current time exceeds m_dwNextUpdateTime
bool Regulator::isReady()
{
	//if a regulator is instantiated with a zero freq then it goes into
	//stealth mode (doesn't regulate)
	if (isEqual(0.0, m_dUpdatePeriod)) return true;

	//if the regulator is instantiated with a negative freq then it will
	//never allow the code to flow
	if (m_dUpdatePeriod < 0) return false;

	unsigned int CurrentTime = timeGetTime();

	//the number of milliseconds the update period can vary per required
	//update-step. This is here to make sure any multiple clients of this class
	//have their updates spread evenly
	static const double UpdatePeriodVariator = 10.0;

	if (CurrentTime >= m_dwNextUpdateTime)
	{
		m_dwNextUpdateTime = (unsigned int)(CurrentTime + m_dUpdatePeriod + RandInRange(-UpdatePeriodVariator, UpdatePeriodVariator));

		return true;
	}

	return false;
}