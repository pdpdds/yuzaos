#pragma once
#include "SkyQueue.h"
#include "I_GUIEngine.h"

class SkyInputManager
{
public:
	SkyInputManager();
	~SkyInputManager();

	bool Initialize();
	bool GetKeyFromKeyQueue(KEYDATA* pstData);
	bool PutKeyQueue(KEYDATA* pstData);

	bool GetMouseDataFromMouseQueue(BYTE& buttonStatus, int& xpos, int& ypos);
	bool PutMouseueue(MOUSEDATA* pstData);

private:
	SkyQueue* m_pKeyQueue;
	SkyQueue* m_pMouseQueue;

	KEYDATA m_keyQueueBuffer[KEY_MAXQUEUECOUNT];
	MOUSEDATA m_mouseQueueBuffer[MOUSE_MAXQUEUECOUNT];

	HANDLE m_mutex;
};

