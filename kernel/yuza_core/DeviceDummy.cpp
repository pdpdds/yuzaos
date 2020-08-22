#include "DeviceDummy.h"
#include "diskio.h"
#include <HardDisk.h>

extern "C" DRESULT IDE_disk_read(BYTE *buff, DWORD sector, UINT count);
extern "C" DRESULT IDE_disk_write(const BYTE *buff, DWORD sector, UINT count);


#ifdef SKY_EMULATOR
#else
extern HardDiskHandler* g_pHDDHandler2;
#endif

DeviceDummy::DeviceDummy()
{	
#ifdef SKY_EMULATOR
#else
	g_pHDDHandler2 = new HardDiskHandler();
	g_pHDDHandler2->Initialize();
#endif
}


DeviceDummy::~DeviceDummy()
{
}

int DeviceDummy::Read(off_t offset, void *ptr, size_t size)
{
	return IDE_disk_read((BYTE *)ptr, offset/512, size/512);
	 
}

int DeviceDummy::Write(off_t offset, const void *ptr, size_t size)
{
	return IDE_disk_write((BYTE *)ptr, offset / 512, size / 512);
}

int DeviceDummy::Control(int op, void*)
{
	return 0;
}

Device* DummyInstantiate()
{
	return new DeviceDummy();
}
