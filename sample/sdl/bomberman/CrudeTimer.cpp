#include "CrudeTimer.h"
#include "utils.h"

#ifdef _WIN32
#pragma comment(lib, "winmm.lib")

#include <windows.h>
#endif

CrudeTimer* CrudeTimer::Instance()
{
  static CrudeTimer instance;

  return &instance;
}

CrudeTimer::CrudeTimer()
{ 
	m_dStartTime = timeGetTime() * 0.001; 
}

double CrudeTimer::GetCurTime()
{
	return timeGetTime() * 0.001 - m_dStartTime;
}