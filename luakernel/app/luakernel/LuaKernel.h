#pragma once
#include <minwindef.h>
#include "I_GUIEngine.h"
#include <Keyboard.h>
#include <Mouse.h>
#include <InputQueue.h>

class LuaKernel : public I_GUIEngine
{
public:
	LuaKernel() {}
	~LuaKernel() {}

	virtual bool Initialize() override;
	virtual void SetLinearBuffer(LinearBufferInfo& linearBufferInfo) override;
	virtual bool PutKeyboardQueue(KEYDATA* pData) override;
	virtual bool PutMouseQueue(MOUSEDATA* pData) override;

	virtual void Update(float deltaTime) override;
	virtual bool Run() override;
	virtual bool Print(QWORD taskId, char* pMsg) override;
	virtual char GetCh() override;
	virtual bool Clear() override;
};