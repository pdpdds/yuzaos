#include "windef.h"
#include "PhysicalMemoryManager.h"
#include "memory.h"
#include <stringdef.h>
#include <Constants.h>
#include "SkyConsole.h"
#include <BootParams.h>

namespace PhysicalMemoryManager
{	
	UINT32	m_usedBlocks = 0;

	UINT32	m_totalMemory = 0;

	//이용할 수 있는 최대 블럭 갯수
	UINT32	m_maxBlocks = 0;

	//비트맵 배열, 각 비트는 메모리 블럭을 표현, 비트맵처리
	UINT32*	m_pMemoryMap = 0;
	UINT32	m_memoryMapSize = 0;

	unsigned int GetFreeFrame();
	unsigned int GetFreeFrames(size_t size);

	void MarkMemBitmap(UINT32 base, size_t size);
	void UnmarkMemBitmap(UINT32 base, size_t size);
	void InitMemoryMap(multiboot_info_t* pBootInfo);

	void SetBit(int bit);
	void UnsetBit(int bit);
	bool TestMemoryMap(int bit);
	UINT32 GetTotalMemory();

	void Initialize(UINT32 moduleEnd, UINT memorySize, multiboot_info_t* pBootInfo)
	{
		SkyConsole::Print("Physical Memory Manager Init..\n");
		m_totalMemory = memorySize;
		m_maxBlocks = memorySize / PMM_BLOCK_SIZE;
		m_pMemoryMap = (UINT32*)PAGE_ALIGN_UP(moduleEnd);
		InitMemoryMap(pBootInfo);

		Dump();
	}

	int GetBitmapEnd()
	{
		return (int)((char*)m_pMemoryMap + m_memoryMapSize);
	}

	UINT32 GetTotalMemory() 
	{ 
		return m_totalMemory; 
	}

	//8번째 메모리 블럭이 사용중임을 표시하기 위해 1로 세팅하려면 배열 첫번째 요소(4바이트) 바이트의 8번째 비트에 접근해야 한다
	void SetBit(int bit)
	{
		m_pMemoryMap[bit / 32] |= (1 << (bit % 32));
	}

	//8번째 메모리 블럭을 사용할 수 있음을 표시하기 위해 0으로 세팅하려면 배열 첫번째 요소(4바이트) 바이트의 8번째 비트에 접근해야 한다
	//00000001000000000000000000000000
	//11111110111111111111111111111111
	//AND 연산자에 의해 8번째 비트를 제외한 비트는 모두 원래 값을 유지한다
	void UnsetBit(int bit)
	{
		m_pMemoryMap[bit / 32] &= ~(1 << (bit % 32));
	}

	void* AllocBlock() 
	{
		if (m_maxBlocks - m_usedBlocks <= 0)
			return NULL;

		unsigned int frame = GetFreeFrame();

		if (frame == -1)
			return NULL;

		SetBit(frame);
		//SkyConsole::Print("free frame : 0x%x\n", frame);

		UINT32 baseIOMemory = (UINT32)m_pMemoryMap + m_memoryMapSize;
		UINT32 addr = frame * PMM_BLOCK_SIZE;
		
		m_usedBlocks++;

		return (void*)addr;
	}

	void FreeBlock(void* pa) 
	{
		UINT32 addr = (uint32_t)pa;	
		UINT32 frame = addr / PMM_BLOCK_SIZE;

		UnsetBit(frame);

		m_usedBlocks--;
	}

	
	void* AllocBlocks(size_t size)
	{
		if (m_maxBlocks - m_usedBlocks <= size)
		{
			LOG_FATAL("AllocBlocks Fail!! %d %d %d\n", m_maxBlocks, m_usedBlocks, size);
		}

		int frame = GetFreeFrames(size);

		//연속된 빈 블럭들이 존재하지 않는다.
		if (frame == -1)
		{
			LOG_FATAL("AllocBlocks frame!!\n");	
		}

		for (uint32_t i = 0; i < size; i++)
			SetBit(frame + i);


		UINT32 baseIOMemory = (UINT32)m_pMemoryMap + m_memoryMapSize;
		UINT32 addr = frame * PMM_BLOCK_SIZE;
	
		m_usedBlocks += size;

		return (void*)addr;
	}

	void FreeBlocks(void* pa, size_t size) {

		UINT32 addr = (UINT32)pa;
		int frame = addr / PMM_BLOCK_SIZE;

		for (UINT32 i = 0; i < size; i++)
			UnsetBit(frame + i);

		m_usedBlocks -= size;
	}

	UINT32 GetUsedBlockCount() {

		return m_usedBlocks;
	}

	UINT32	GetFreeMemory()
	{
		return (m_maxBlocks - m_usedBlocks) * PMM_BLOCK_SIZE;
	}

	//해당 비트가 세트되어 있는지 되어 있지 않은지를 체크한다	
	bool TestMemoryMap(int bit)
	{
		return (m_pMemoryMap[bit / 32] & (1 << (bit % 32))) > 0;
	}

	//비트가 0인 프레임 인덱스를 얻어낸다(사용할 수 있는 빈 블럭 비트 인덱스)
	unsigned int GetFreeFrame()
	{
		for (uint32_t i = 0; i < m_maxBlocks / PMM_BITS_PER_INDEX; i++)
		{
			if (m_pMemoryMap[i] != 0xffffffff)
				for (unsigned int j = 0; j < PMM_BITS_PER_INDEX; j++)
				{
					unsigned int bit = 1 << j;
					if ((m_pMemoryMap[i] & bit) == 0)
						return i * PMM_BITS_PER_INDEX + j;
				}
		}

		return 0xffffffff;
	}

	//연속된 빈 프레임(블럭)들을 얻어낸다
	unsigned int GetFreeFrames(size_t size)
	{
		if (size == 0)
			return 0xffffffff;

		if (size == 1)
			return GetFreeFrame();

	
		for (uint32_t i = 0; i < m_maxBlocks / PMM_BITS_PER_INDEX; i++)
		{			
			if (m_pMemoryMap[i] != 0xffffffff)
			{				
				for (unsigned int j = 0; j < PMM_BITS_PER_INDEX; j++)
				{
					unsigned int bit = 1 << j;
					if ((m_pMemoryMap[i] & bit) == 0)
					{
						
						unsigned int startingBit = i * PMM_BITS_PER_INDEX + j;

						// 연속된 빈 프레임의 갯수를 증가시킨다.
						uint32_t free = 0;
						for (uint32_t count = 0; count < size; count++)
						{
							//메모리맵을 벗어나는 상황
							if (startingBit + count >= m_maxBlocks)
								return 0xffffffff;

							if (TestMemoryMap(startingBit + count) == false)
								free++;
							else 
								break;

							//연속된 빈 프레임들이 존재한다. 시작 비트 인덱스는 startingBit
							if (free == size)
								return startingBit;
						}
					}
				}
			}
		}

		return 0xffffffff;
	}

	void MarkMemBitmap(UINT32 base, size_t size)
	{
		int startBlock = base / PMM_BLOCK_SIZE;
		int endBlock = (base + size) / PMM_BLOCK_SIZE;
		int remain = (base + size) % PMM_BLOCK_SIZE;
		if (remain > 0)
			endBlock += 1;

		for (int i = startBlock; i < endBlock; i++) 
		{			
			SetBit(i);
			m_usedBlocks++;			
		}
	}

	void UnmarkMemBitmap(UINT32 base, size_t size)
	{
		int startBlock = base / PMM_BLOCK_SIZE;
		int endBlock = (base + size) / PMM_BLOCK_SIZE;
		int remain = (base + size) % PMM_BLOCK_SIZE;
		if (remain > 0)
			endBlock += 1;

		for (int i = startBlock; i < endBlock; i++)
		{
			UnsetBit(i);
			m_usedBlocks--;	
		}
	}

	void Dump()
	{
		SkyConsole::Print("Total Memory (%d MB)\n", m_maxBlocks * PMM_BLOCK_SIZE / MEGA_BYTES);
		SkyConsole::Print("Memory Map Address : 0x%x\n", m_pMemoryMap);
		SkyConsole::Print("Memory Map Size : (%d BYTE)\n", m_memoryMapSize);
		SkyConsole::Print("Max Block Count : %d\n", m_maxBlocks);
		SkyConsole::Print("Used Block Count : %d\n", m_usedBlocks);
	}

	void InitMemoryMap(multiboot_info_t* pBootInfo)
	{
		//메모리 비트맵 크기 계산. UINT 배열 요소 하나당 PMM_BITS_PER_INDEX 비트 표현 가능
		UINT arrayCount = m_maxBlocks / PMM_BITS_PER_INDEX;

		if (m_maxBlocks % PMM_BITS_PER_INDEX)
			arrayCount += 1;

		m_memoryMapSize = arrayCount * sizeof(UINT32);

		//모든 메모리 블럭들이 사용중에 있다고 설정한다.	
		unsigned char flag = 0xff;
		memset((char*)m_pMemoryMap, flag, m_memoryMapSize);
		m_usedBlocks = m_maxBlocks;

		uint32_t mmapEntryNum = pBootInfo->mmap_length / sizeof(multiboot_memory_map_t);
		multiboot_mmap_entry* mmapAddr = (multiboot_mmap_entry*)pBootInfo->mmap_addr;

		for (uint32_t i = 0; i < mmapEntryNum; i++)
		{
			uint64_t areaStart = (uint64_t)mmapAddr[i].baseAddressLower | ((uint64_t)mmapAddr[i].baseAddressHigher << 32);
			uint64_t areaEnd = areaStart + ((uint64_t)mmapAddr[i].lengthLower | ((uint64_t)mmapAddr[i].lengthHigher << 32));

			if (areaEnd < (uint64_t)0x100000)
				continue;

			switch (mmapAddr[i].type)
			{
			case MEMORY_REGION_AVAILABLE:
			{
				//사용할 수 있는 공간을 UnSet한다.
				SkyConsole::Print("MEMORY_REGION_AVAILABLE : 0x%q 0x%x 0x%x\n", areaStart, mmapAddr[i].lengthLower, mmapAddr[i].lengthHigher);
				UnmarkMemBitmap(areaStart, (UINT32)(areaEnd - areaStart));
			}
			break;
			}
		}	
		//OMB 부터 메모리맵 영역까지 Set한다.
		MarkMemBitmap(0, (UINT32)m_pMemoryMap + m_memoryMapSize);
	}
}