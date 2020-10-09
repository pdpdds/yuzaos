#pragma once
#include "windef.h"
#include "stdint.h"
#include "SkyRenderer32.h"
#include "I_GUIEngine.h"

class UnicodeFont;
class HangulProcessor;

class GUIConsole : public I_GUIEngine
{
public:
	GUIConsole();
	~GUIConsole();

	virtual bool Initialize() override;
	virtual void Update(float deltaTime) override;
	virtual void SetLinearBuffer(LinearBufferInfo& linearBufferInfo) override;

	virtual bool PutKeyboardQueue(KEYDATA* pData) override;
	virtual bool PutMouseQueue(MOUSEDATA* pData) override;

	bool Run() override;
	bool Print(QWORD taskId, char* pMsg) override;
	bool Clear() override;
	virtual char GetCh() { return 0; }

	bool Print2(char* pMsg);
	void GetNewLine();
	void GetCommandForGUI(char* commandBuffer, int bufSize);
	
	ULONG GetWidth() { return m_width; }

private:
	void FillRect(int x, int y, int w, int h, int col);
	void PutCursor();
	void PutPixel(ULONG x, ULONG y, ULONG col);
	void PutPixel(ULONG i, ULONG col);
	void PutPixel(ULONG i, unsigned char r, unsigned char g, unsigned char b);
	ULONG GetPixel(ULONG i);
	ULONG GetBPP();
	

private:
	ULONG* m_pVideoRamPtr;
	int m_width;
	int m_height;
	int m_bpp;

	int m_yPos;
	int m_xPos;
	UnicodeFont* m_pUnicodeFont;
	HangulProcessor* m_pHangulProcessor;
};