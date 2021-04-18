#pragma once
#include "FileSysAdaptor.h"

class USBAdaptor : public FileSysAdaptor
{
public:
	USBAdaptor(char* deviceName);
	virtual ~USBAdaptor();

	bool Initialize(I_FileSystem* pFileSystem, void* arg) override;
};

