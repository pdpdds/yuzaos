#pragma once
#include "Device.h"
#include "ff.h"

class DeviceDummy : public Device 
{
public:
	DeviceDummy();
	virtual ~DeviceDummy();

	virtual int Read(off_t offset, void *ptr, size_t size);
	virtual int Write(off_t offset, const void *ptr, size_t size);
	virtual int Control(int op, void*) override;
};

