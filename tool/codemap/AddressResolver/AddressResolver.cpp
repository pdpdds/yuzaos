#include <iostream>
#include "codemap.h"
#include "mydbg2.h"

bool ResolveAddress(MyCoffDbg2Stt* pInfo, DWORD dwResolveAddress, DWORD dwRelocBase)
{
	if (dwResolveAddress == 0)
		return false;

	int		nLinenumber = -1;
	char	pFuncName[48] = { 0, };
	char	szSrcFile[48] = { 0, };

	nLinenumber = get_file_func_lineno(pInfo, szSrcFile, pFuncName, dwResolveAddress - dwRelocBase);

	if (nLinenumber < 0)
		return false;

	printf("%s: %s (%d)\n", szSrcFile, &pFuncName[1], nLinenumber);

		return true;
}

int main(int argc, char* argv[])
{
	printf("YUZA OS Address Resolver (c) Copyright 2020 by Juhang Park\n");

	if (argc <= 3)
	{
		printf("USAGE : AddressReolver <DBG_FILE> [resolveAddress] [baseAddress]\n");
		return 0;
	}

	MyCoffDbg2Stt* pInfo = load_mydbg2_info(argv[1]);

	if (pInfo == 0)
		return false;

	DWORD resolveAddress = dwHexValue(argv[2]);
	DWORD baseAddress = dwHexValue(argv[3]);

	ResolveAddress(pInfo, resolveAddress, baseAddress);
	
	return 0;
}