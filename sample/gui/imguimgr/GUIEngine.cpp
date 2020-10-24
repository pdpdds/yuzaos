#include "GUIEngine.h"
#include "windef.h"
#include "string.h"
#include "memory.h"
#include "InputQueue.h"
#include "sample.h"
#include "Mouse.h"
#include "Keyboard.h"


extern "C" __declspec(dllexport) I_GUIEngine* GetGUIEngine()
{
	I_GUIEngine* pEngine = new GUIEngine();
	return pEngine;
}


GUIEngine::GUIEngine()
{
	
}


GUIEngine::~GUIEngine()
{
}

bool GUIEngine::Initialize() 
{	
	kInitializeMouse();
	kInitializeKeyboard();
	return true;
}

extern int samplef(void* buffer);

void GUIEngine::Update(float deltaTime) 
{
	//wnd_update();
	samplef(m_linearBufferInfo.pBuffer);	
	//kUpdate(m_linearBufferInfo.isDirectVideoBuffer);
}

void GUIEngine::SetLinearBuffer(LinearBufferInfo& linearBufferInfo)
{
	m_linearBufferInfo = linearBufferInfo;
	//init_lfb(linearBufferInfo.pBuffer, linearBufferInfo.width, linearBufferInfo.height, linearBufferInfo.depth, linearBufferInfo.type);
}

extern QUEUE gs_stKeyQueue;
extern QUEUE gs_stMouseQueue;
bool GUIEngine::PutKeyboardQueue(KEYDATA* pData)
{
	return kPutQueue(&gs_stKeyQueue, pData);
}

bool  GUIEngine::PutMouseQueue(MOUSEDATA* pData)
{
	return kPutQueue(&gs_stMouseQueue, pData);
}


bool GUIEngine::Run()
{
	Update(0);

	return true;
}

bool GUIEngine::Print(QWORD taskId, char* pMsg)
{
	return true;
}

bool GUIEngine::Clear()
{
	return true;
}


//int colorStatus[] = { 0x00FF0000, 0x0000FF00, 0x0000FF };
//ULONG* lfAb = (ULONG*)SkyGUISystem::GetInstance()->GetVideoRamInfo()._pVideoRamPtr;
//SampleFillRect(lfAb, 0, 0, 20, 20, 0x00ff0000);