#include <orangeos.h>
#include "PEImage32.h"
#include "ProcessUtil32.h"
#include "SkyConsole.h"

//32비트 PE파일 이미지 유효성 검사
bool ValidatePEImage(void* image)
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

	//if ((ntHeaders->OptionalHeader.ImageBase < 0x400000) || (ntHeaders->OptionalHeader.ImageBase > 0x80000000))
		//return false;

	/* only support 32 bit optional header format */
	if (ntHeaders->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
		return false;

	//유효한 32비트 PE 파일이다.
	return true;
}

bool FindModuleEntry(const char* szFileName, char* buf, KernelInfo* pInfo)
{
	if (!ValidatePEImage(buf)) {
		SkyConsole::Print("Invalid PE Format!! %s\n", szFileName);
		return false;
	}

	IMAGE_DOS_HEADER* dosHeader = 0;
	IMAGE_NT_HEADERS* ntHeaders = 0;

	SkyConsole::Print("Valid PE Format %s\n", szFileName);

	dosHeader = (IMAGE_DOS_HEADER*)buf;
	ntHeaders = (IMAGE_NT_HEADERS*)(dosHeader->e_lfanew + (uint32_t)buf);	

	pInfo->_kernelBase = ntHeaders->OptionalHeader.ImageBase;
	pInfo->_kernelEntry = (uint32_t)ntHeaders->OptionalHeader.AddressOfEntryPoint + pInfo->_kernelBase;
	
	SkyConsole::Print("image base : 0x%x, entry : 0x%x\n", pInfo->_kernelBase, pInfo->_kernelEntry);

	return 	true;
}

