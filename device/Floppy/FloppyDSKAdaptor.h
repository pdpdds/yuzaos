#pragma once
#include "FileSysAdaptor.h"


class FloppyDSKAdaptor : public FileSysAdaptor
{
public:
	FloppyDSKAdaptor(char* deviceName);
	virtual ~FloppyDSKAdaptor();

	bool Initialize(I_FileSystem* pFileSystem, void* arg) override;
	
private:
	bool InitializeWin32();
};

