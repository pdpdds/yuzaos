#include "HardDisk.h"
#include "memory.h"
#include <stringdef.h>
#include <WinError.h>
#include <systemcall_impl.h>

extern "C" int _outp(unsigned short, int);
extern "C" unsigned long _outpd(unsigned int, int);
extern "C" unsigned short _outpw(unsigned short, unsigned short);
extern "C" int _inp(unsigned short);
extern "C" unsigned short _inpw(unsigned short);
extern "C" unsigned long _inpd(unsigned int shor);
extern "C" unsigned char inports(unsigned short port, unsigned short* buffer, int count);

void OutPortByte(ushort port, uchar value)
{
	_outp(port, value);
}

void OutPortWord(ushort port, ushort value)
{
	_outpw(port, value);
}

void OutPortDWord(ushort port, unsigned int value)
{
	_outpd(port, value);
}

long InPortDWord(unsigned int port)
{
	return _inpd(port);
}

uchar InPortByte(ushort port)
{

	return (uchar)_inp(port);
}

ushort InPortWord(ushort port)
{
	return _inpw(port);
}


//�߰ߵ� �ϵ��ũ ������ �����Ѵ�.
BYTE HardDiskHandler::GetTotalDevices()
{
	return (BYTE)HDDs.Count();
}

//---------------------------------------------------------------------
//        This function returns the description of the last error
//---------------------------------------------------------------------
char * HardDiskHandler::GetLastError(BYTE errorCode)
{
	switch (errorCode)
	{
	case HDD_NO_ERROR:
		return "No Error";
	case HDD_NOT_FOUND:
		return "HDD Not Found";
	case HDD_CONTROLLER_BUSY:
		return "Device Controller Busy";
	case HDD_DATA_NOT_READY:
		return "Device Data Not Ready";
	case HDD_DATA_COMMAND_NOT_READY:
		return "Device not ready";
	default:
		return "Undefined Error";
	}
}

char * HardDiskHandler::GetLastError()
{
	return GetLastError(m_lastError);
}

//����̽��� ������ �߻����� ��� ���� ���������� ���� �о���δ�.
BYTE ReadErrorRegister(BYTE deviceController)
{
	BYTE Status = InPortByte(IDE_Con_IOBases[deviceController][0] + IDE_CB_STATUS);
	if ((Status & 0x80) == 0 && (Status & 0x1)) //busy bit=0 and err bit=1
	{
		Status = InPortByte(IDE_Con_IOBases[deviceController][0] + IDE_CB_ERROR);
		return Status;
	}
	else
		return 0;
}

//����̽��� �����͸� �����ϰų� ���� �� �ִ� �غ� �Ǿ����� Ȯ���Ѵ�.
//deviceController : �׽�Ʈ�� ����̽� ��Ʈ�ѷ��� �ε��� ��ȣ
//����̽��� �����Ͱ��� ó���� �� �� �ִ� �غ� �Ǿ����� TRUE�� �׷��� ������ FALSE�� �����Ѵ�.
BOOLEAN IsDeviceDataReady(int deviceController, DWORD waitUpToms = 0, BOOLEAN checkDataRequest = TRUE)
{
	UINT32 Time1, Time2;
	Time1 = Syscall_GetTickCount();
	do
	{
		UINT16 PortID = IDE_Con_IOBases[deviceController][0] + IDE_CB_STATUS;
		BYTE Status = InPortByte(PortID);
		if ((Status & 0x80) == 0) //Checking BSY bit, because DRDY bit is valid only when BSY is zero
		{
			if (Status & 0x40) //checking DRDY is set
				if (checkDataRequest) // if DataRequest is also needed
				{
					if (Status & 0x8) // DRQ bit set
					{
						return TRUE;
					}
				}
				else
				{
					return TRUE;
				}
		}
		Time2 = Syscall_GetTickCount();
	} while ((Time2 - Time1) < waitUpToms);

	return FALSE;
}

//�־��� ����̽� ��Ʈ�ѷ��� ����� �� �ִ��� �������� üũ�Ѵ�.
BOOLEAN HardDiskHandler::IsDeviceControllerBusy(int DeviceController, int WaitUpToms)
{
	UINT32 Time1, Time2;
	Time1 = Syscall_GetTickCount();
	do {

		UINT16 PortID = IDE_Con_IOBases[DeviceController][0] + IDE_CB_STATUS;
		BYTE Status = InPortByte(PortID);
		if ((Status & 0x80) == 0) //BSY bit 
			return FALSE;
		Time2 = Syscall_GetTickCount();
	} while ((Time2 - Time1) <= (UINT32)WaitUpToms);

	return TRUE;
}

//������ ����̽��� ����Ʈ���� �����Ѵ�.
BYTE HardDiskHandler::DoSoftwareReset(UINT16 DeviceController)
{
	BYTE DeviceControl = 4; //SRST bit ���� ���������� SRST ��Ʈ �ʵ忡 ���� ����
	OutPortByte(IDE_Con_IOBases[DeviceController][0] + IDE_CON_DEVICE_CONTROL, DeviceControl);
	DeviceControl = 0;      //���� ���������� SRST ��Ʈ�� Ŭ����
	OutPortByte(IDE_Con_IOBases[DeviceController][0] + IDE_CON_DEVICE_CONTROL, DeviceControl);

	return InPortByte(IDE_Con_IOBases[DeviceController][0] + IDE_CB_ERROR);
}
BOOLEAN HardDiskHandler::IsRemovableDevice(BYTE * DPF)
{
	return !(HDDs.Item((char *)DPF)->DeviceID[0] & 0x70);
}
BOOLEAN HardDiskHandler::IsRemovableMedia(BYTE * DPF)
{
	return HDDs.Item((char *)DPF)->DeviceID[0] & 0x80;
}

HardDiskHandler::HardDiskHandler()
{

}

/*
�ʱ�ȭ �޼ҵ� : ��� ����̽� ��Ʈ�ѷ��� Ȯ���ؼ� �̿��� �� �ִ��� üũ�Ѵ�.
1) ����̽� ��Ʈ�ѷ��� Busy ��Ʈ�� Ȯ���Ѵ�. �� ���� �����Ǹ� �ش� ����̽� ��Ʈ�ѷ��� ����� �� ����.
2) ����̽��� �����ϴ� Ŀ�ǵ带 ������.
3) Ư���ð� ��⵿�� Busy ��Ʈ�� Ŭ����Ǹ� ����̽� ��Ʈ�ѷ��� ������ �� �ִ�.
4) ���� �������͸� �д´�.
	a) ��Ʈ���� 0�̸� ������ ��ũ�� ��ġ�� ���� �ǹ�
	b) ��Ʈ���� 7�̸� �����̺� ��ũ ��ġ���� ����
5) DEV_HEAD �������Ϳ� ������ ��Ʈ���� �����Ѵ�.
6) 50ns ���� ����Ѵ�.
7) ����̽� Ŀ�ǵ带 ������.
8) ����̽��κ��� 512����Ʈ �������� �޴´�.
*/
void HardDiskHandler::Initialize()
{
	char strKey[3] = "H0"; //�ϵ��ũ ID
	
	//�ƹ��� ������ ���� �ʴ� �ϵ��ũ �ڵ鷯������ ���Ǹ� �ؾ� �Ѵ�.
	Syscall_SetDriverInterruptVector(32 + 14, 0);
	Syscall_SetDriverInterruptVector(32 + 15, 0);
	
	//Collection ����ü �߰��� �ϵ��ũ ���� ����Ʈ�� �����Ѵ�.
	HDDs.Initialize();	

	//����̽� ��Ʈ�ѷ��� ���� �ϵ��ũ�� ã�´�.
	for (int DeviceController = 0; DeviceController < IDE_CONTROLLER_NUM; DeviceController++)
	{
		DoSoftwareReset(DeviceController); //����Ʈ���� ����
		if (IsDeviceControllerBusy(DeviceController, 1000)) //����̽� ��Ʈ�ѷ��� ����� �� ������ �н��Ѵ�.
			continue;
		
		//����̽� ���� ��û�� �Ѵ�.
		OutPortByte(IDE_Con_IOBases[DeviceController][0] + IDE_CB_COMMAND, IDE_COM_EXECUTE_DEVICE_DIAGNOSTIC);
				
		BYTE result = InPortByte(IDE_Con_IOBases[DeviceController][0] + IDE_CB_ERROR);
		for (BYTE device = 0; device < 1; device++)         //�����Ϳ� �����̺� ��ũ�� ���� ������ ����.
		{
			UINT16 DeviceID_Data[512], j;
			
			//if (device == 0 && !(result & 1))
				//continue;

			if (device == 1 && (result & 0x80))
				continue;

			//����̽� IO�� �����ϴٸ�
			if (device == 1)
				OutPortByte(IDE_Con_IOBases[DeviceController][0] + IDE_CB_DEVICE_HEAD, 0x10); //Setting 4th bit(count 5) to set device as 1
			else
				OutPortByte(IDE_Con_IOBases[DeviceController][0] + IDE_CB_DEVICE_HEAD, 0x0);

			Syscall_Sleep(50);

			//����̽� ���� ��û
			OutPortByte(IDE_Con_IOBases[DeviceController][0] + IDE_CB_COMMAND, IDE_COM_IDENTIFY_DEVICE);
			if (!IsDeviceDataReady(DeviceController, 600, TRUE)) //����̽� ������ ä���������� ����Ѵ�.
			{
				printf("Data not ready %d\n", DeviceController);
				continue;
			}

			//����̽��� ���� 512����Ʈ ������ �о���δ�.
			for (j = 0; j < 256; j++)
				DeviceID_Data[j] = InPortWord(IDE_Con_IOBases[DeviceController][0] + IDE_CB_DATA);
			
			//HDD ��� ����
			HDDInfo * newHDD = (HDDInfo *)malloc(sizeof(HDDInfo));
			if (newHDD == NULL)
			{
				printf("HDD Initialize :: Allocation failed\n");
				return;
			}

			//HDD ��忡 ����̽� ������ ����Ѵ�.
			newHDD->IORegisterIdx = DeviceController;
			memcpy(newHDD->DeviceID, DeviceID_Data, 512);
			newHDD->DeviceNumber = device;
			newHDD->LastError = 0;

			newHDD->BytesPerSector = 512; 

			newHDD->CHSCylinderCount = DeviceID_Data[1];
			newHDD->CHSHeadCount = DeviceID_Data[3];
			newHDD->CHSSectorCount = DeviceID_Data[6];

			if (DeviceID_Data[10] == 0)
				strcpy(newHDD->SerialNumber, "N/A");
			else
				for (j = 0; j < 20; j += 2)
				{
					newHDD->SerialNumber[j] = DeviceID_Data[10 + (j / 2)] >> 8;
					newHDD->SerialNumber[j + 1] = (DeviceID_Data[10 + (j / 2)] << 8) >> 8;
				}
			if (DeviceID_Data[23] == 0)
				strcpy(newHDD->FirmwareRevision, "N/A");
			else
				for (j = 0; j < 8; j += 2)
				{
					newHDD->FirmwareRevision[j] = DeviceID_Data[23 + (j / 2)] >> 8;
					newHDD->FirmwareRevision[j + 1] = (DeviceID_Data[23 + (j / 2)] << 8) >> 8;
				}

			if (DeviceID_Data[27] == 0)
				strcpy(newHDD->ModelNumber, "N/A");
			else
				for (j = 0; j < 20; j += 2)
				{
					newHDD->ModelNumber[j] = DeviceID_Data[27 + (j / 2)] >> 8;
					newHDD->ModelNumber[j + 1] = (DeviceID_Data[27 + (j / 2)] << 8) >> 8;
				}
			newHDD->LBASupported = DeviceID_Data[49] & 0x200;
			newHDD->DMASupported = DeviceID_Data[49] & 0x100;

			UINT32 LBASectors = DeviceID_Data[61];
			LBASectors = LBASectors << 16;
			LBASectors |= DeviceID_Data[60];			
			newHDD->LBACount = LBASectors;
			HDDs.Add(newHDD, strKey);

			printf("DeviceId : %x, %s\n", device, newHDD->ModelNumber);
			strKey[1]++; //�� �ϵ��ũ ��带 ���� �ϵ��ũ ID�� �����Ѵ�.
		}
	}
}

HardDiskHandler::~HardDiskHandler()
{
	HDDs.Clear();
}
HDDInfo * HardDiskHandler::GetHDDInfo(BYTE * DPF)
{

	HDDInfo * getHDD, *retHDD = (HDDInfo *)malloc(sizeof(HDDInfo));
	getHDD = HDDs.Item((char *)DPF);
	if (getHDD == NULL)
	{
		m_lastError = HDD_NOT_FOUND;
		return NULL;
	}
	memcpy(retHDD, getHDD, sizeof(HDDInfo));
	return retHDD;
}
/* ���ͷκ��� �����͸� �о���δ�(CHS ���)
1) HDDInfo ��ü���� ��´�.
2) ����̽��� ����� �� �ִ��� Ȯ���Ѵ�.
3) ����̽� ��Ʈ�� �����Ѵ�.
4) ����̽��� ������ Ŀ�ǵ带 �޾Ƶ��� �غ� �Ǿ����� Ȯ���Ѵ�.
5) ���� Ʈ��, ��Ÿ������ �����Ѵ�.
6) �б� Ŀ�ǵ带 ������.
7) ����̽��� ������ ������ �� �� �ִ� �غ� �Ǿ����� Ȯ���Ѵ�.
8) �����͸� �б� ���� ������ �������͸� �д´�.
*/
BYTE HardDiskHandler::ReadSectors(BYTE * DPF, UINT16 StartCylinder, BYTE StartHead, BYTE StartSector, BYTE NoOfSectors, BYTE * buffer, BOOLEAN WithRetry)
{
	HDDInfo * pHDDInfo;
	BYTE DevHead, StartCylHigh = 0, StartCylLow = 0;

	//�ϵ��ũ ���̵�� ���� �ϵ��ũ������ ����.
	pHDDInfo = HDDs.Item((char *)"H0");
	if (pHDDInfo == NULL)
	{
		m_lastError = HDD_NOT_FOUND;
		return HDD_NOT_FOUND;
	}

	if (pHDDInfo->DeviceNumber == 0)
		DevHead = StartHead | 0xA0;
	else
		DevHead = StartHead | 0xB0;

	//����̽��� �غ�ɶ� ���� ����Ѵ�.
	if (IsDeviceControllerBusy(pHDDInfo->IORegisterIdx, 1 * 60))
	{
		m_lastError = HDD_CONTROLLER_BUSY;
		return HDD_CONTROLLER_BUSY;
	}

	//����̽��� ������ Ŀ�ǵ带 �޾Ƶ��� �غ� �Ǿ����� Ȯ���Ѵ�.
	OutPortByte(IDE_Con_IOBases[pHDDInfo->IORegisterIdx][0] + IDE_CB_DEVICE_HEAD, DevHead);

	if (!IsDeviceDataReady(pHDDInfo->IORegisterIdx, 1 * 60, FALSE))
	{
		m_lastError = HDD_DATA_COMMAND_NOT_READY;
		return HDD_DATA_COMMAND_NOT_READY;
	}

	StartCylHigh = StartCylinder >> 8;
	StartCylLow = (StartCylinder << 8) >> 8;

	//�о���� �������� ��ġ�� �����Ѵ�. �Ǹ��� ��ġ, ���� ������ġ, �о���� ������ ��
	OutPortByte(IDE_Con_IOBases[pHDDInfo->IORegisterIdx][0] + IDE_CB_CYLINDER_HIGH, StartCylHigh);
	OutPortByte(IDE_Con_IOBases[pHDDInfo->IORegisterIdx][0] + IDE_CB_CYLINDER_LOW, StartCylLow);
	OutPortByte(IDE_Con_IOBases[pHDDInfo->IORegisterIdx][0] + IDE_CB_SECTOR, StartSector);
	OutPortByte(IDE_Con_IOBases[pHDDInfo->IORegisterIdx][0] + IDE_CB_SECTOR_COUNT, NoOfSectors);

	OutPortByte(IDE_Con_IOBases[pHDDInfo->IORegisterIdx][0] + IDE_CB_COMMAND, WithRetry ? IDE_COM_READ_SECTORS_W_RETRY : IDE_COM_READ_SECTORS);

	//��û�� ���ͼ���ŭ �����͸� �о���δ�.
	for (BYTE j = 0; j < NoOfSectors; j++)
	{
		//����̽��� �����Ͱ� �غ�Ǿ��°�?
		if (!IsDeviceDataReady(pHDDInfo->IORegisterIdx, 1 * 60, TRUE))
		{
			m_lastError = HDD_DATA_NOT_READY;
			return HDD_DATA_NOT_READY;
		}

		// �� ����Ʋ ���� ���� ũ���� 512����Ʈ�� ���ۿ� ����� �� �ִ�.
		for (UINT16 i = 0; i < (pHDDInfo->BytesPerSector) / 2; i++)
		{
			UINT16 w = 0;
			BYTE l, h;

			//2����Ʈ�� �д´�.
			w = InPortWord(IDE_Con_IOBases[pHDDInfo->IORegisterIdx][0] + IDE_CB_DATA);
			l = (w << 8) >> 8;
			h = w >> 8;
			
			//2����Ʈ�� ����.
			buffer[(j * (pHDDInfo->BytesPerSector)) + (i * 2)] = l;
			buffer[(j * (pHDDInfo->BytesPerSector)) + (i * 2) + 1] = h;
		}
	}
	return HDD_NO_ERROR;
}

// ���ͷκ��� �����͸� �о���δ�(LBA ���)
// �о���̴� ��ƾ�� CHS ���� �����ϴ�.
BYTE HardDiskHandler::ReadSectors(BYTE * DPF, UINT32 StartLBASector, BYTE NoOfSectors, BYTE * Buffer, BOOLEAN WithRetry)
{
	HDDInfo * HDD;
	BYTE LBA0_7, LBA8_15, LBA16_23, LBA24_27;

	HDD = HDDs.Item((char *)DPF);
	if (HDD == NULL)
	{
		m_lastError = HDD_NOT_FOUND;
		return HDD_NOT_FOUND;
	}
	LBA0_7 = (StartLBASector << 24) >> 24;
	LBA8_15 = (StartLBASector << 16) >> 24;
	LBA16_23 = (StartLBASector << 8) >> 24;
	LBA24_27 = (StartLBASector << 4) >> 28;

	if (HDD->DeviceNumber == 0)
		LBA24_27 = LBA24_27 | 0xE0;
	else
		LBA24_27 = LBA24_27 | 0xF0;

	if (IsDeviceControllerBusy(HDD->IORegisterIdx, 1 * 60))
	{
		m_lastError = HDD_CONTROLLER_BUSY;
		return HDD_CONTROLLER_BUSY;
	}

	OutPortByte(IDE_Con_IOBases[HDD->IORegisterIdx][0] + IDE_CB_DEVICE_HEAD, LBA24_27);

	if (!IsDeviceDataReady(HDD->IORegisterIdx, 1 * 60, FALSE))
	{
		m_lastError = HDD_DATA_COMMAND_NOT_READY;
		return HDD_DATA_COMMAND_NOT_READY;
	}

	OutPortByte(IDE_Con_IOBases[HDD->IORegisterIdx][0] + IDE_CB_LBA_16_23, LBA16_23);
	OutPortByte(IDE_Con_IOBases[HDD->IORegisterIdx][0] + IDE_CB_LBA_8_15, LBA8_15);
	OutPortByte(IDE_Con_IOBases[HDD->IORegisterIdx][0] + IDE_CB_LBA_0_7, LBA0_7);
	OutPortByte(IDE_Con_IOBases[HDD->IORegisterIdx][0] + IDE_CB_SECTOR_COUNT, NoOfSectors);

	OutPortByte(IDE_Con_IOBases[HDD->IORegisterIdx][0] + IDE_CB_COMMAND, WithRetry ? IDE_COM_READ_SECTORS_W_RETRY : IDE_COM_READ_SECTORS);

//��û�� ���ͼ���ŭ �����͸� �д´�.
	for (BYTE j = 0; j < NoOfSectors; j++)
	{
		if (!IsDeviceDataReady(HDD->IORegisterIdx, 1 * 60, TRUE))
		{
			m_lastError = HDD_DATA_NOT_READY;
			return HDD_DATA_NOT_READY;
		}

//20191111
		inports(IDE_Con_IOBases[HDD->IORegisterIdx][0] + IDE_CB_DATA, (ushort*)&Buffer[(j * (HDD->BytesPerSector))], (HDD->BytesPerSector) / 2);
		
		/*for (UINT16 i = 0; i < (HDD->BytesPerSector) / 2; i++)
		{
			UINT16 w = 0;
			BYTE l, h;
			w = InPortWord(IDE_Con_IOBases[HDD->IORegisterIdx][0] + IDE_CB_DATA);
			l = (w << 8) >> 8;
			h = w >> 8;
			Buffer[(j * (HDD->BytesPerSector)) + (i * 2)] = l;
			Buffer[(j * (HDD->BytesPerSector)) + (i * 2) + 1] = h;
		}*/
	}
	return HDD_NO_ERROR;
}


/*���Ϳ� �����͸� ����.
1) HDDInfo ��ü�� ����.
2) ����̽��� ����� �� �ִ��� üũ�Ѵ�.
3) ����̽� ��Ʈ�� �����Ѵ�.
4) ����̽��� ������ Ŀ�ǵ带 �޾Ƶ��� �� �ִ��� üũ�Ѵ�.
5) ���, Ʈ��, ��Ÿ ������ �����Ѵ�.
6) ���� Ŀ�ǵ带 �����Ѵ�.
7) ����̽��� �����͸� ���� �غ� �Ǿ����� üũ�Ѵ�.
8) �����͸� �����ϱ� ���� ������ �������Ϳ� �����͸� ����Ѵ�.
*/
BYTE HardDiskHandler::WriteSectors(BYTE * DPF, UINT16 StartCylinder, BYTE StartHead, BYTE dwStartLBASector, BYTE NoOfSectors, BYTE * lpBuffer, BOOLEAN WithRetry)
{
	HDDInfo * pHDDInfo;
	BYTE LBA0_7, LBA8_15, LBA16_23, LBA24_27;

	pHDDInfo = HDDs.Item((char *)DPF);
	if (pHDDInfo == NULL)
	{
		m_lastError = HDD_NOT_FOUND;
		return HDD_NOT_FOUND;
	}

	LBA0_7 = (dwStartLBASector << 24) >> 24;
	LBA8_15 = (dwStartLBASector << 16) >> 24;
	LBA16_23 = (dwStartLBASector << 8) >> 24;
	LBA24_27 = (dwStartLBASector << 4) >> 28;

	if (pHDDInfo->DeviceNumber == 0)
		LBA24_27 = LBA24_27 | 0xE0;
	else
		LBA24_27 = LBA24_27 | 0xF0;

	if (IsDeviceControllerBusy(pHDDInfo->IORegisterIdx, 400))
	{		
		Syscall_SetLastError(ERROR_BUSY);
		return HDD_CONTROLLER_BUSY;
	}

	OutPortByte(IDE_Con_IOBases[pHDDInfo->IORegisterIdx][0] + IDE_CB_DEVICE_HEAD, LBA24_27);

	if (!IsDeviceDataReady(pHDDInfo->IORegisterIdx, 1000, FALSE))
	{
		Syscall_SetLastError(ERROR_NOT_READY);
		return HDD_DATA_COMMAND_NOT_READY;
	}
	OutPortByte(IDE_Con_IOBases[pHDDInfo->IORegisterIdx][0] + IDE_CB_LBA_16_23, LBA16_23);
	OutPortByte(IDE_Con_IOBases[pHDDInfo->IORegisterIdx][0] + IDE_CB_LBA_8_15, LBA8_15);
	OutPortByte(IDE_Con_IOBases[pHDDInfo->IORegisterIdx][0] + IDE_CB_LBA_0_7, LBA0_7);
	OutPortByte(IDE_Con_IOBases[pHDDInfo->IORegisterIdx][0] + IDE_CB_SECTOR_COUNT, NoOfSectors);

	OutPortByte(IDE_Con_IOBases[pHDDInfo->IORegisterIdx][0] + IDE_CB_COMMAND, IDE_COM_WRITE_SECTORS_W_RETRY);
	for (UINT16 j = 0; j < NoOfSectors; j++)
	{
		if (!IsDeviceDataReady(pHDDInfo->IORegisterIdx, 1000, TRUE))
		{
			Syscall_SetLastError(ERROR_NOT_READY);
			return HDD_DATA_NOT_READY;
		}
		for (UINT16 i = 0; i < pHDDInfo->BytesPerSector / 2; i++)
		{
			OutPortWord(IDE_Con_IOBases[pHDDInfo->IORegisterIdx][0] + IDE_CB_DATA, ((UINT16 *)lpBuffer)[(j * (pHDDInfo->BytesPerSector) / 2) + i]);
		}
	}

	return HDD_NO_ERROR;
}

BYTE HardDiskHandler::WriteSectors(BYTE * DPF, UINT32 dwStartLBASector, BYTE NoOfSectors, BYTE * lpBuffer, BOOLEAN WithRetry)
{
	HDDInfo * pHDDInfo;
	BYTE LBA0_7, LBA8_15, LBA16_23, LBA24_27;
	pHDDInfo = HDDs.Item((char *)DPF);
	if (pHDDInfo == NULL)
	{
		m_lastError = HDD_NOT_FOUND;
		return HDD_NOT_FOUND;
	}

	LBA0_7 = (dwStartLBASector << 24) >> 24;
	LBA8_15 = (dwStartLBASector << 16) >> 24;
	LBA16_23 = (dwStartLBASector << 8) >> 24;
	LBA24_27 = (dwStartLBASector << 4) >> 28;

	if (pHDDInfo->DeviceNumber == 0)
		LBA24_27 = LBA24_27 | 0xE0;
	else
		LBA24_27 = LBA24_27 | 0xF0;

	if (IsDeviceControllerBusy(pHDDInfo->IORegisterIdx, 400))
	{
		Syscall_SetLastError(ERROR_BUSY);
		return HDD_CONTROLLER_BUSY;
	}

	OutPortByte(IDE_Con_IOBases[pHDDInfo->IORegisterIdx][0] + IDE_CB_DEVICE_HEAD, LBA24_27);

	if (!IsDeviceDataReady(pHDDInfo->IORegisterIdx, 1000, FALSE))
	{
		Syscall_SetLastError(ERROR_NOT_READY);
		return HDD_DATA_COMMAND_NOT_READY;
	}
	OutPortByte(IDE_Con_IOBases[pHDDInfo->IORegisterIdx][0] + IDE_CB_LBA_16_23, LBA16_23);
	OutPortByte(IDE_Con_IOBases[pHDDInfo->IORegisterIdx][0] + IDE_CB_LBA_8_15, LBA8_15);
	OutPortByte(IDE_Con_IOBases[pHDDInfo->IORegisterIdx][0] + IDE_CB_LBA_0_7, LBA0_7);
	OutPortByte(IDE_Con_IOBases[pHDDInfo->IORegisterIdx][0] + IDE_CB_SECTOR_COUNT, NoOfSectors);

	OutPortByte(IDE_Con_IOBases[pHDDInfo->IORegisterIdx][0] + IDE_CB_COMMAND, IDE_COM_WRITE_SECTORS_W_RETRY);
	for (UINT16 j = 0; j < NoOfSectors; j++)
	{
		if (!IsDeviceDataReady(pHDDInfo->IORegisterIdx, 1000, TRUE))
		{
			Syscall_SetLastError(ERROR_NOT_READY);
			return HDD_DATA_NOT_READY;
		}
		for (UINT16 i = 0; i < pHDDInfo->BytesPerSector / 2; i++)
		{
			OutPortWord(IDE_Con_IOBases[pHDDInfo->IORegisterIdx][0] + IDE_CB_DATA, ((UINT16 *)lpBuffer)[(j * (pHDDInfo->BytesPerSector) / 2) + i]);
		}
	}

	return HDD_NO_ERROR;
}

//Ư�� ����̽��� �Ķ���� ������ ����.
UINT16 HardDiskHandler::GetDeviceParameters(BYTE * DPF, BYTE * pBuffer)
{
	VFS_IO_PARAMETER deviceInfo;

	HDDInfo * getHDD;
	getHDD = this->HDDs.Item((char *)DPF);
	if (getHDD == NULL)
	{
		this->m_lastError = HDD_NOT_FOUND;
		return HDD_NOT_FOUND;
	}

	deviceInfo.Cylinder = getHDD->CHSCylinderCount;
	deviceInfo.Head = getHDD->CHSHeadCount;
	deviceInfo.Sector = getHDD->CHSSectorCount;
	deviceInfo.LBASector = getHDD->LBACount;
	memcpy(pBuffer, &deviceInfo, sizeof(VFS_IO_PARAMETER));

	return HDD_NO_ERROR;
}

//�־��� ����̽� ��Ʈ�ѷ��� �����ϰ� �� ����� �����Ѵ�.
//����̽� ��Ʈ�ѷ��� ������ DoSoftwareReset �޼ҵ忡�� �����Ѵ�.
BYTE HardDiskHandler::Reset(BYTE * DPF)
{
	HDDInfo * getHDD;
	getHDD = this->HDDs.Item((char *)DPF);
	if (getHDD == NULL)
	{
		this->m_lastError = HDD_NOT_FOUND;
		return 0;
	}
	return this->DoSoftwareReset(getHDD->IORegisterIdx);

}

//�ּ� ��� ���� CHS => LBA
UINT32 HardDiskHandler::CHSToLBA(BYTE * DPF, UINT32 Cylinder, UINT32 Head, UINT32 Sector)
{
	HDDInfo * getHDD;
	getHDD = this->HDDs.Item((char *)DPF);

	return (Sector - 1) + (Head*getHDD->CHSSectorCount) + (Cylinder * (getHDD->CHSHeadCount + 1) * getHDD->CHSSectorCount);
}

//�ּ� ��� ���� LBA => CHS
void HardDiskHandler::LBAToCHS(BYTE * DPF, UINT32 LBA, UINT32 * Cylinder, UINT32 * Head, UINT32 * Sector)
{
	HDDInfo * getHDD;
	getHDD = this->HDDs.Item((char *)DPF);

	*Sector = ((LBA % getHDD->CHSSectorCount) + 1);
	UINT32 CylHead = (LBA / getHDD->CHSSectorCount);
	*Head = (CylHead % (getHDD->CHSHeadCount + 1));
	*Cylinder = (CylHead / (getHDD->CHSHeadCount + 1));
}
