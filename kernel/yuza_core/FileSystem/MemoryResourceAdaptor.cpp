#include "MemoryResourceAdaptor.h"
#include <string>
#include <SystemAPI.h>

MemoryResourceAdaptor::MemoryResourceAdaptor(char* deviceName)
	: FileSysAdaptor(deviceName)
{
}

MemoryResourceAdaptor::~MemoryResourceAdaptor()
{
}

bool MemoryResourceAdaptor::Initialize(I_FileSystem* pFileSystem, void* arg)
{
	m_pFileSystem = pFileSystem;
	kDebugPrint("Memory Resource Disk Init Complete\n");
	return true;
}
