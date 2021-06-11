#include "SkyVirtualInput.h"
#include "BuildOption.h"
#include "SkyGUISystem.h"

SkyVirtualInput::SkyVirtualInput()
{
}
 
SkyVirtualInput::~SkyVirtualInput()
{
}

bool SkyVirtualInput::PutKeyboardQueue(KEYDATA* pData)
{
	SkyGUISystem::GetInstance()->PutKeyboardQueue(pData);
	return false;
}

bool SkyVirtualInput::Print(QWORD taskId, char* str)
{
	SkyGUISystem::GetInstance()->Print(taskId, str);
	return true;
}

bool SkyVirtualInput::PutMouseQueue(MOUSEDATA* pData)
{
	SkyGUISystem::GetInstance()->PutMouseQueue(pData);
	
	return false;
}

extern void KernelSoftwareInterrupt();
void SkyVirtualInput::SoftwareInterrupt()
{
	KernelSoftwareInterrupt();
}
