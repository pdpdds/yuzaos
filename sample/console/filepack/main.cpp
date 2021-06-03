#include "FilePack.h"

int main(int argc, char* argv[])
{
	/*CFilePack* pPack = new CFilePack();

	if (pPack->OpenFile("zelda.zip") == true)
	{
		if (pPack->OpenFileFromZip("zelda.bmp"))
		{
			pPack->Store("zelda.bmp");
		}
	}

	delete pPack;

	TCHAR		buffer[MAX_PATH];

	DWORD result = GetCurrentDirectory(MAX_PATH, &buffer[0]);
	lstrcpy(&buffer[result], _T("\\MyDir"));

	CFileDirPack DirPack;

	DirPack.DirPack(_T("test.zip"), buffer, result);*/

	Orange::CFilePackSys ArchiveManager;
	if (ArchiveManager.Initialize() == TRUE)
		ArchiveManager.UnPackFile("test.zip");

	return 0;
}
