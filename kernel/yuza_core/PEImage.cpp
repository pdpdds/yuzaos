#include <minwindef.h>
#include <PEImage.h>
#include <PEFile.h>
#include <SystemAPI.h>
#include <memory.h>
#include <ModuleManager.h>
#include <platformapi.h>

extern PlatformAPI g_platformAPI;

bool RelocatePE(DWORD imageBase, UINT64 peSize, DWORD relocationBase)
{
	PeImgStt					pe;
	DWORD* pT, dwTemp;
	DWORD						dwLastImageByte;
	DWORD						dwDebugPosition;
	LONG						lBase, lDestBase;
	DWORD						dwBuiltInV86Lib, dwBuiltInV86LibSize;

	pe.pBuff = (UCHAR*)imageBase;
	pe.pDosHd = (MY_IMAGE_DOS_HEADER*)(pe.pBuff);
	// 기타 베이스 오프셋을 설정한다.
	pe.lPeBase = pe.lIfBase = pe.pDosHd->e_lfanew;
	pe.lIoBase = pe.lPeBase + sizeof(MY_IMAGE_FILE_HEADER);
	// 각 HEADER의 주소를 계산한다.
	pe.pPeHd = (MY_IMAGE_PE_HEADER*)&pe.pBuff[pe.pDosHd->e_lfanew];
	pe.pIfHd = (MY_IMAGE_FILE_HEADER*)pe.pPeHd;
	pe.pIoHd = (MY_IMAGE_OPTIONAL_HEADER*)&pe.pBuff[pe.pDosHd->e_lfanew + sizeof(MY_IMAGE_FILE_HEADER)];

	// 첫번째 섹션의 헤더위치를 구한다.
	lBase = pe.lIoBase + sizeof(MY_IMAGE_OPTIONAL_HEADER);

	// 옮겨갈 곳 계산.
	lDestBase = 0;
	//pe.pBase = (UCHAR*)relocationBase;  // 옮겨갈 곳의 주소.
	pe.pBase = (UCHAR*)new char[peSize + 4096];

	// 2002-12-13 커널 이미지 들어갈 곳을 0으로 클리어.
	memset(pe.pBase, 0, 512);

	// 섹션 바로 윗부분까지는 그냥 옮겨버린다.
	memcpy(pe.pBase, pe.pBuff, lBase);

	for (int nI = 0; nI < pe.pIfHd->NumberOfSections && nI < BDF_MAX_PE_SECTION - 1; nI++)
	{
		// 섹션 헤더의 위치	 계산
		pe.sect[nI].lBase = lBase;
		// 섹션 헤더의 포인터 계산
		pe.sect[nI].pPtr = (MY_IMAGE_SECTION_HEADER*)&pe.pBuff[lBase];
		// 섹션명 복사
		memset(pe.sect[nI].szName, 0, 8);
		memcpy(pe.sect[nI].szName, pe.sect[nI].pPtr, 8);
		pT = (DWORD*)pe.sect[nI].szName;

		// 섹션명을 화면에 복사한다.
		kDebugPrint("%s\n", pe.sect[nI].szName);

		// 필요에 따라 섹션의 타입을 결정한다.
		pe.sect[nI].nType = nI;

		// 섹션의 바디만 정해진 위치에 옮겨 놓으면 된다.
		dwTemp = (DWORD)&pe.pBuff[pe.sect[nI].pPtr->PointerToRawData];
		memset(&pe.pBase[pe.sect[nI].pPtr->VirtualAddress], 0, pe.sect[nI].pPtr->VirtualSize);  // 0으로 클리어.
		memcpy(&pe.pBase[pe.sect[nI].pPtr->VirtualAddress], (char*)dwTemp, pe.sect[nI].pPtr->SizeOfRawData);
		dwTemp += pe.sect[nI].pPtr->SizeOfRawData;
		dwTemp = (DWORD)(((dwTemp + 511) / 512) * 512);

		lBase += sizeof(MY_IMAGE_SECTION_HEADER);

		// 이미지의 마지막 바이트 위치
		dwLastImageByte = (DWORD)&pe.pBase[pe.sect[nI].pPtr->VirtualAddress] + pe.sect[nI].pPtr->VirtualSize;
	}

	// 디버깅 정보를 옮긴다. (VC6 with CODEMAP utility)
	if (pe.pIoHd->dd_Debug_dwVAddr != 0)
	{
		char* pS;
		DWORD* pX;

		// dd_Debug_dwVAddr은 그냥 오프셋으로 사용한다.
		pS = (char*)&pe.pBuff[pe.pIoHd->dd_Debug_dwVAddr];
		pX = (DWORD*)pS;

		if (pX[0] == (DWORD)0x46464F43)
		{
			dwDebugPosition = dwLastImageByte;
			dwLastImageByte += pe.pIoHd->dd_Debug_dwSize;

			// 디버깅 정보를 복사한다.
			memcpy((UCHAR*)dwDebugPosition, pS, pe.pIoHd->dd_Debug_dwSize);
		}
		else
			dwDebugPosition = 0;
	}

	// 도스 헤더 바로 다음 위치에 MAIGIC값과 V86Lib 오프셋이 있는지 확인.
	dwBuiltInV86Lib = 0;
	dwBuiltInV86LibSize = 0;
	/*pT = (DWORD*)((DWORD)pe.pDosHd + sizeof(MY_IMAGE_DOS_HEADER));
	if (pT[0] == V86PARAM_MAGIC)
	{
		dwBuiltInV86Lib = dwLastImageByte;
		dwBuiltInV86LibSize = pT[2]; // 사이즈
									 // DOS STUB를 옮긴다.
		memcpy((BYTE*)dwBuiltInV86Lib, (BYTE*)pT[1] + (DWORD)pe.pDosHd, dwBuiltInV86LibSize);
		dwLastImageByte += dwBuiltInV86LibSize;
	}*/

	// 이미지의 마지막 바이트 오프셋을 4096으로 올림한다.
	dwLastImageByte = (DWORD)(((dwLastImageByte + 4095) / 4096) * 4096);

	return true;
}


bool FixIAT(void* image)
{
	IMAGE_DOS_HEADER* dosHeader = 0;
	IMAGE_NT_HEADERS* ntHeaders = 0;

	dosHeader = (IMAGE_DOS_HEADER*)image;
	if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		return false;

	if (dosHeader->e_lfanew == 0)
		return false;

	//NT Header 체크
	ntHeaders = (IMAGE_NT_HEADERS*)(dosHeader->e_lfanew + (uint32_t)image);
	if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
		return false;

	/* only supporting for i386 archs */
	if (ntHeaders->FileHeader.Machine != IMAGE_FILE_MACHINE_I386)
		return false;

	/* only support 32 bit executable images */
	if (!(ntHeaders->FileHeader.Characteristics & (IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_32BIT_MACHINE)))
		return false;

	/*
	Note: 1st 4 MB remains idenitity mapped as kernel pages as it contains
	kernel stack and page directory. If you want to support loading below 1MB,
	make sure to move these into kernel land
	*/

	//로드되는 프로세스의 베이스 주소는 0x00400000다. 
	//비쥬얼 스튜디오에서 속성=> 링커 => 고급의 기준주소 항목에서 확인 가능하다
	if ((ntHeaders->OptionalHeader.ImageBase < 0x400000) || (ntHeaders->OptionalHeader.ImageBase > 0x80000000))
		return false;

	/* only support 32 bit optional header format */
	if (ntHeaders->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
		return false;

	auto importDir = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

	if (!importDir.VirtualAddress || !importDir.Size)
		return false;

	auto importDescriptor = PIMAGE_IMPORT_DESCRIPTOR(ntHeaders->OptionalHeader.ImageBase + importDir.VirtualAddress);
	auto fixIATCount = 0;

	for (; importDescriptor->Name; importDescriptor++)
	{
		kDebugPrint("OriginalFirstThunk: %x\n", importDescriptor->OriginalFirstThunk);
		kDebugPrint("     TimeDateStamp: %x\n", importDescriptor->TimeDateStamp);
		kDebugPrint("    ForwarderChain: %x\n", importDescriptor->ForwarderChain);
		//if (!IsBadReadPtr((char*)image + importDescriptor->Name, 2))
		kDebugPrint("              Name: %x %s\n", importDescriptor->Name, (char*)image + importDescriptor->Name);

		if (strcmp("win32stub.dll", (char*)image + importDescriptor->Name) == 0)
			continue;

		char* dllName = (char*)image + importDescriptor->Name;


#if SKY_EMULATOR_DLL
		HMODULE hwnd = g_platformAPI._processInterface.sky_GetModuleHandle(dllName);

		pInitializeDll InitializeDll = (pInitializeDll)ModuleManager::GetInstance()->GetModuleFunction(hwnd, "InitializeDll");
		if (InitializeDll != nullptr)
			InitializeDll();

		FixIAT((void*)hwnd);
		continue;
#else
		void* hwnd = 0;
		hwnd = (void*)ModuleManager::GetInstance()->GetSystemPE(dllName);
		if (!hwnd)
			hwnd = ModuleManager::GetInstance()->LoadPE(dllName);

		/*pInitializeDll InitializeDll = (pInitializeDll)ModuleManager::GetInstance()->GetModuleFunction(hwnd, "InitializeDll");
		if (InitializeDll != nullptr)
			InitializeDll();*/

		auto thunkData = PIMAGE_THUNK_DATA32(ULONG_PTR(image) + importDescriptor->FirstThunk);

		PIMAGE_THUNK_DATA32 pthunk;
		if (importDescriptor->OriginalFirstThunk == 0)
			pthunk = PIMAGE_THUNK_DATA32(ULONG_PTR(image) + importDescriptor->FirstThunk);
		else
			pthunk = PIMAGE_THUNK_DATA32(ULONG_PTR(image) + importDescriptor->OriginalFirstThunk);
		PIMAGE_THUNK_DATA32 nextthunk;
		for (int i = 0; pthunk->u1.Function != 0; i++, pthunk++)
		{
			nextthunk = PIMAGE_THUNK_DATA32(ULONG_PTR(image) + importDescriptor->FirstThunk);
			if ((pthunk->u1.Ordinal & IMAGE_ORDINAL_FLAG) == 0)
			{
				PIMAGE_IMPORT_BY_NAME pname = (PIMAGE_IMPORT_BY_NAME)((PCHAR)image + pthunk->u1.AddressOfData);

				void* p = ModuleManager::GetInstance()->GetModuleFunction(hwnd, (char*)pname->Name);

				if (p)
				{
					nextthunk[i].u1.Function = reinterpret_cast<DWORD>(p);
					//kDebugPrint("Function: %x %s\n", nextthunk[i].u1.Function, (char*)pname->Name);
					fixIATCount++;
				}
			}
		}
#endif
	}

	kDebugPrint("%d imports parsed!\n", fixIATCount);

	return true;
}