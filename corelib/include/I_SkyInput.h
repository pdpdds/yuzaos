#pragma once

#ifndef QWORD
typedef long long			QWORD;
#endif

#define MOUSE_LBUTTONDOWN   0x01
#define MOUSE_RBUTTONDOWN   0x02
#define MOUSE_MBUTTONDOWN   0x04

#pragma pack( push, 1 )

typedef struct tag_KEYDATA
{	
	unsigned char bScanCode;
	unsigned char bASCIICode;	
	unsigned char bFlags;
} KEYDATA;

typedef struct tag_MouseData
{	
	unsigned char bButtonStatusAndFlag;	
	int bXMovement;
	int bYMovement;
	// 상대좌표값인지 절대좌표값인지를 나타내는 플래그
	bool bAbsoluteCoordinate;
} MOUSEDATA;

typedef struct tag_MouseState
{
	int iByteCount;
	MOUSEDATA stCurrentData;
} MOUSESTATE;

#pragma pack( pop )

#ifdef __cplusplus
class I_SkyInput
{
public:
	virtual bool PutKeyboardQueue(KEYDATA* pData) = 0;
	virtual bool PutMouseQueue(MOUSEDATA* pData) = 0;
	virtual bool Print(QWORD taskId, char* str) { return false; }
	virtual void SoftwareInterrupt() {}
};
#endif