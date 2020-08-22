#pragma once

#include "BackingStore.h"
#include "Mutex.h"

class SwapSpace : public BackingStore 
{
public:
	SwapSpace();
	virtual ~SwapSpace();
	virtual bool HasPage(off_t) override;
	virtual int Read(off_t, void*, int) override;
	virtual int Write(off_t, const void*) override;
	virtual off_t Commit(off_t size) override;
	static int SwapOn(const char path[], off_t size);

private:
	static int AllocSwapSpace();
	static void FreeSwapSpace(int offset);

	int fChunkArraySize;
	struct SwapChunk *fChunkArray;
	Mutex fLock;
	int fCommittedSize;
	static int *fSwapMap;
	static int fSwapDevice;
	static off_t fSwapChunkCount;
	static off_t fFreeSwapChunks;
	static Mutex fSwapLock;
};