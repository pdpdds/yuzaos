#pragma once
#include "FileSysAdaptor.h"

class MemoryResourceAdaptor : public FileSysAdaptor
{
public:
	MemoryResourceAdaptor(char* deviceName);
	~MemoryResourceAdaptor();

	virtual bool Initialize(I_FileSystem* pFileSystem, void* arg) override;

};

