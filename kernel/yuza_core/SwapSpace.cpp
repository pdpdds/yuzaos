#pragma once
// 
#include "windef.h"
#include <stringdef.h>
#include "SwapSpace.h"
#include "memory.h"
#include "cpu_asm.h"
#include "Debugger.h"

const int kChunkSize = PAGE_SIZE * 4;

struct SwapChunk {
	int offset : (32 - (kChunkSize / PAGE_SIZE)), // 1 based
		alloc : (kChunkSize / PAGE_SIZE);
};

int *SwapSpace::fSwapMap = 0;
int SwapSpace::fSwapDevice = -1;
off_t SwapSpace::fSwapChunkCount = 0;
Mutex SwapSpace::fSwapLock;
off_t SwapSpace::fFreeSwapChunks = 0x7fffffff;	// Hack

SwapSpace::SwapSpace()
	:	fChunkArraySize(0),
		fChunkArray(0),
		fCommittedSize(0)
{
}

SwapSpace::~SwapSpace()
{
	for (int chunkIndex = 0; chunkIndex < fChunkArraySize; chunkIndex++)
		if (fChunkArray[chunkIndex].offset)
			FreeSwapSpace(fChunkArray[chunkIndex].offset);

	delete [] fChunkArray;
	fSwapLock.Lock();
	fFreeSwapChunks += fCommittedSize;
	fSwapLock.Unlock();
}

bool SwapSpace::HasPage(off_t offset)
{
	int arrayOffset = offset / kChunkSize;
	int chunkOffset = offset % kChunkSize;
	if (arrayOffset >= fChunkArraySize)
		return false;

	return (fChunkArray[arrayOffset].alloc & (1 << (chunkOffset / PAGE_SIZE))) != 0;
}

int SwapSpace::Read(off_t offset, void *va, int)
{
	kprintf("<");
	if (offset / kChunkSize > fChunkArraySize)
		kPanic("reading unswapped page");

	off_t deviceOffset = (fChunkArray[offset / kChunkSize].offset - 1) * kChunkSize + (offset % kChunkSize);
	fseek((FILE*)fSwapDevice, deviceOffset, SEEK_SET);
	return fread(va, 1, PAGE_SIZE, (FILE*)fSwapDevice);
}

int SwapSpace::Write(off_t offset, const void *va)
{
	if (fSwapDevice < 0)
		kPanic("no swap file opened");

	fLock.Lock();
	int arrayOffset = offset / kChunkSize;
	int chunkOffset = offset % kChunkSize;
	if (arrayOffset >= fChunkArraySize) {
		SwapChunk *newChunkArray = new SwapChunk[arrayOffset + 1];
		if (fChunkArray) {
			memcpy(newChunkArray, fChunkArray, fChunkArraySize * sizeof(SwapChunk));
			delete [] fChunkArray;
		}

		fChunkArray = newChunkArray;
		memset(fChunkArray + fChunkArraySize, 0, (arrayOffset - fChunkArraySize + 1) * sizeof(SwapChunk));
		fChunkArraySize = arrayOffset + 1;
	}

	if (fChunkArray[arrayOffset].offset == 0)
		fChunkArray[arrayOffset].offset = AllocSwapSpace();

	fChunkArray[arrayOffset].alloc |= (1 << (chunkOffset / PAGE_SIZE));
	off_t deviceOffset = (fChunkArray[offset / kChunkSize].offset - 1) * kChunkSize +
		(offset % kChunkSize);
	kprintf(">");
	ASSERT(HasPage(offset));
	fLock.Unlock();
	fseek((FILE*)fSwapDevice, deviceOffset, SEEK_SET);
	return fwrite(va, 1, PAGE_SIZE, (FILE*)fSwapDevice);
}

off_t SwapSpace::Commit(off_t size)
{
	fSwapLock.Lock();
	int delta = (fCommittedSize - size) / kChunkSize;
	if (delta > 0 && delta > fFreeSwapChunks) {
		fSwapLock.Unlock();
		return fCommittedSize;	// Fail
	}

	fFreeSwapChunks += (fCommittedSize - size) / kChunkSize;
	fCommittedSize = size;
	fSwapLock.Unlock();
	return size;
}

int SwapSpace::SwapOn(const char file[], off_t size)
{
	fSwapDevice = (int)fopen(file, "rw");
	if (fSwapDevice <= 0) {
		kprintf("Error opening swap device\n");
		return fSwapDevice;
	}

	fSwapMap = new int[size / kChunkSize / 32 + 1];
	memset(fSwapMap, 0, size / kChunkSize / 8);
	fSwapChunkCount = size / kChunkSize;

	return 0;
}

int SwapSpace::AllocSwapSpace()
{
	fSwapLock.Lock();
	int wordCount = fSwapChunkCount / 32;
	for (int wordIndex = 0; wordIndex < wordCount; wordIndex += 32) {
		if (fSwapMap[wordIndex] == static_cast<int>(0xffffffff))
			continue;

		for (int bitIndex = 0; bitIndex < 32; bitIndex++) {
			if ((fSwapMap[wordIndex] & (1 << bitIndex)) == 0) {
				fSwapMap[wordIndex] |= (1 << bitIndex);
				fSwapLock.Unlock();
				return wordIndex + bitIndex + 1;
			}
		}
	}

	kPanic("Swap space exhausted");
	return -1;
}

void SwapSpace::FreeSwapSpace(int offset)
{
	fSwapLock.Lock();
	fSwapMap[offset / 32] &= ~(1 << (offset % 32));
	fSwapLock.Unlock();
}
