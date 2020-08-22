#pragma once
#include "windef.h"
#include "Collect.h"
#include "HardDiskIO.h"

class HardDiskHandler
{
public:
	HardDiskHandler();
	~HardDiskHandler();

	void Initialize();

	BYTE ReadSectors(BYTE* DPF, UINT16 StartCylinder, BYTE StartHead, BYTE StartSector, BYTE NoOfSectors, BYTE* buffer, BOOLEAN WithRetry = TRUE);
	BYTE ReadSectors(BYTE* DPF, UINT32 StartLBASector, BYTE NoOfSectors, BYTE* buffer, BOOLEAN WithRetry = TRUE);
	BYTE WriteSectors(BYTE* DPF, UINT16 StartCylinder, BYTE StartHead, BYTE StartSector, BYTE NoOfSectors, BYTE* Buffer, BOOLEAN WithRetry = TRUE);
	BYTE WriteSectors(BYTE* DPF, UINT32 StartLBASector, BYTE NoOfSectors, BYTE* Buffer, BOOLEAN WithRetry = TRUE);

	BYTE GetTotalDevices();
	HDDInfo* GetHDDInfo(BYTE* DPF);
	char* GetLastError();

protected:
	//주소모드 변환
	UINT32 CHSToLBA(BYTE* DPF, UINT32 Cylinder, UINT32 Head, UINT32 Sector);
	void LBAToCHS(BYTE* DPF, UINT32 LBA, UINT32* Cylinder, UINT32* Head, UINT32* Sector);
	BYTE Reset(BYTE* DPF);

	char* GetLastError(BYTE errorCode);
	UINT16 GetDeviceParameters(BYTE* DPF, BYTE* Buffer);

	BOOLEAN IsRemovableDevice(BYTE* DPF);
	BOOLEAN IsRemovableMedia(BYTE* DPF);
	BOOLEAN IsDeviceControllerBusy(int DeviceController, int WaitUpToms = 0);

private:
	Collection <HDDInfo*> HDDs;
	void(*InterruptHandler)();
	static BYTE DoSoftwareReset(UINT16 deviceController);
	BYTE m_lastError;
};

extern HardDiskHandler* g_pHDDHandler;
