#include "LuaKernel.h"
#include "stdint.h"
#include "PlatformAPI.h"
#include "memory.h"
#include "systemcall_impl.h"
#include <string.h>

extern "C" __declspec(dllexport) I_GUIEngine * GetGUIEngine()
{
	return new LuaKernel();
}
extern bool HandleInterrupt(unsigned char scanCode);
extern int HandleTerminalWrite(char* msg, int len);
extern bool lua_main(uint8 * pFrameBuffer, int width, int height, int bpp);

extern QUEUE gs_stKeyQueue;
extern QUEUE gs_stMouseQueue;

bool LuaKernel::Initialize()
{
	kInitializeKeyboard();
	kInitializeMouse();

	return true;
}

void LuaKernel::SetLinearBuffer(LinearBufferInfo& linearBufferInfo)
{
	m_linearBufferInfo = linearBufferInfo;
}

char LuaKernel::GetCh()
{
	KEYDATA keyData;

	if (kGetKeyFromKeyQueue(&keyData) == true)
	{
		//if ((keyData.bFlags & KEY_FLAGS_DOWN) == true)
		{
//Fix me
			if (keyData.bScanCode == 'Y')
				keyData.bScanCode = 13;

			return keyData.bScanCode;
		}
	}

	return 0;
}

DWORD WINAPI LuaInputProc(LPVOID parameter)
{
	LuaKernel* pKernel = (LuaKernel *)parameter;

	while (1)
	{
		char c= pKernel->GetCh();

		if (c)
			HandleInterrupt(c);

		Syscall_Sleep(0);
	}

	return 0;
}

bool LuaKernel::Run()
{
	uint8* frameBuffer = (uint8*)m_linearBufferInfo.pBuffer;
	int width = m_linearBufferInfo.width;
	int height = m_linearBufferInfo.height;
	int bpp = m_linearBufferInfo.depth / 8;
	
	Syscall_CreateThread(LuaInputProc, "InputProc", this, 16);

	lua_main(frameBuffer, width, height, bpp);

	return true;
}

void LuaKernel::Update(float deltaTime)
{

}

bool LuaKernel::Clear()
{
	return true;
}

bool LuaKernel::Print(QWORD taskId, char* pMsg)
{
	int len = strlen(pMsg);

	if(len == 0)
		return false;

	return HandleTerminalWrite(pMsg, len) != 0;
}

bool LuaKernel::PutKeyboardQueue(KEYDATA* pData)
{
	return kPutQueue(&gs_stKeyQueue, pData);
}

bool LuaKernel::PutMouseQueue(MOUSEDATA* pData)
{
	return kPutQueue(&gs_stMouseQueue, pData);
}