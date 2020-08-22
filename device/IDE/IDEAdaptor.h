#pragma once
#include "./FileSysAdaptor.h"

class IDEAdaptor : public FileSysAdaptor
{
public:
	IDEAdaptor(char* deviceName);
	virtual ~IDEAdaptor();

	void PrintHDDInfo();

	bool Initialize(I_FileSystem* pFileSystem, void* arg) override;
	bool InitializeWin32();
};