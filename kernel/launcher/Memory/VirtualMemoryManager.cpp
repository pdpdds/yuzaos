#include "VirtualMemoryManager.h"
#include "PhysicalMemoryManager.h"
#include <stringdef.h>
#include "memory.h"
#include "X86Arch.h"
#include <stringdef.h>
#include "intrinsic.h"
#include "PlatformAPI.h"
#include <Constants.h>
#include "SkyConsole.h"
#include <BootParams.h>

extern BootParams* g_pBootParams;
extern DWORD* _hyperSpacePTE;


namespace VirtualMemoryManager
{
	//! current directory table

	PageDirectory*		_kernel_directory = 0;
	PageDirectory*		_cur_directory = 0;	

	//Internal Method
	void MapPhysicalAddressToVirtualAddresss(PageDirectory* dir, uint32_t virt, uint32_t phys, uint32_t flags);
	void UnmapPhysicalAddress(PageDirectory* dir, uint32_t virt);
	void UnmapPageTable(PageDirectory* dir, uint32_t virt);
	//페이지 디렉토리를 생성한다.
	PageDirectory* CreatePageDirectory();
	//페이지 테이블을 생성한다. 페이지 테이블의 크기는 4K다.
	bool CreatePageTable(PageDirectory* dir, uint32_t virt, uint32_t flags);

	//Not Used
	//페이지를 할당한다.
	bool AllocPage(PTE* e);
	//페이지를 회수한다.
	void FreePage(PTE* e);

	bool Initialize(int identityPageCount)
	{
		SkyConsole::Print("Virtual Memory Manager Init..\n");

		bool result = SetupKernelPageDirectory(identityPageCount);

		if (false == result)
			return false;

		return true;
	}

	//가상 주소와 매핑된 물리 주소를 얻어낸다.
	uint32_t GetPhysicalAddress(PageDirectory* dir, uint32_t va)
	{
		PDE* pd = dir->m_entries;
		PDE pde = pd[PAGE_DIRECTORY_INDEX(va)];
		PTE* pt = (PTE*)(pde & ~0xfff);

		if (pde == 0)
			return NULL;

		PTE pte = pt[PAGE_TABLE_INDEX(va)];
		if (pte == 0)
			return NULL;

		uint32_t pa = pte & ~0xfff;
		uint32_t offset = PAGE_GET_OFFSET(va);

		return pa + offset;
	}

	//페이지 디렉토리 엔트리 인덱스가 0이 아니면 이미 페이지 테이블이 존재한다는 의미
	bool CreatePageTable(PageDirectory* dir, uint32_t va, uint32_t flags)
	{
		PDE* pd = dir->m_entries;
		if (pd[PAGE_DIRECTORY_INDEX(va)] == 0)
		{
			void* pPageTable = PhysicalMemoryManager::AllocBlock();
			if (pPageTable == nullptr)
				return false;	

			//20181130			
			memset(pPageTable, 0, sizeof(PageTable));
			pd[PAGE_DIRECTORY_INDEX(va)] = ((uint32_t)pPageTable) | flags;
		}
		return true;
	}

	//PDE나 PTE의 플래그는 같은 값을 공유
	//가상주소를 물리 주소에 매핑
	void MapPhysicalAddressToVirtualAddresss(PageDirectory* dir, uint32_t virt, uint32_t phys, uint32_t flags)
	{
		int flag = DisableInterrupts();
		PDE* pageDir = dir->m_entries;				

		if (pageDir[virt >> 22] == 0)
		{
			CreatePageTable(dir, virt, flags);
		}
		
		uint32_t mask = (uint32_t)(~0xfff);
		uint32_t* pageTable = (uint32_t*)(pageDir[virt >> 22] & mask);

		pageTable[virt << 10 >> 10 >> 12] = phys | flags;
		
		RestoreInterrupts(flag);
	}

	void FreePageDirectory(PageDirectory* dir)
	{		
		PDE* pageDir = dir->m_entries;
		for (int i = 0; i < PAGES_PER_DIRECTORY; i++)
		{
			PDE& pde = pageDir[i];

			if (pde != 0)
			{
				/* get mapped frame */
				void* frame = (void*)(pageDir[i] & 0x7FFFF000);
				PhysicalMemoryManager::FreeBlock(frame);
				pde = 0;
			}
		}		
	}

	void UnmapPageTable(PageDirectory* dir, uint32_t virt)
	{
		PDE* pageDir = dir->m_entries;
		if (pageDir[virt >> 22] != 0) {

			/* get mapped frame */
			void* frame = (void*)(pageDir[virt >> 22] & 0x7FFFF000);

			/* unmap frame */
			PhysicalMemoryManager::FreeBlock(frame);
			pageDir[virt >> 22] = 0;
		}
	}


	void UnmapPhysicalAddress(PageDirectory* dir, uint32_t virt)
	{
		PDE* pagedir = dir->m_entries;
		if (pagedir[virt >> 22] != 0)
			UnmapPageTable(dir, virt);
	}

	PageDirectory* CreatePageDirectory()
	{
		PageDirectory* dir = NULL;

		/* allocate page directory */
		dir = (PageDirectory*)PhysicalMemoryManager::AllocBlock();
		if (!dir)
			return NULL;

		//memset(dir, 0, sizeof(PageDirectory));

		return dir;
	}

	bool AllocPage(PTE* e)
	{
		void* p = PhysicalMemoryManager::AllocBlock();

		if (p == NULL)
			return false;

		PageTableEntry::SetFrame(e, (uint32_t)p);
		PageTableEntry::AddAttribute(e, I86_PTE_PRESENT);

		return true;
	}

	void FreePage(PTE* e)
	{

		void* p = (void*)PageTableEntry::GetFrame(*e);
		if (p)
			PhysicalMemoryManager::FreeBlock(p);

		PageTableEntry::DelAttribute(e, I86_PTE_PRESENT);
	}

	//페이지 디렉토리 생성. 
	//페이지 디렉토리는 1024개의 페이지테이블을 가진다
	//1024 * 1024(페이지 테이블 엔트리의 개수) * 4K(프레임의 크기) = 4G	
	
	bool SetupKernelPageDirectory(int identiyPageCount)
	{					
		_kernel_directory = (PageDirectory*)PhysicalMemoryManager::AllocBlocks(sizeof(PageDirectory) / PMM_BLOCK_SIZE);
		memset(_kernel_directory, 0, sizeof(PageDirectory));

		uint32_t frame = 0x00000000;
		uint32_t virt = 0x00000000;

		//페이지 테이블을 생성
		for (int i = 0; i < identiyPageCount; i++)
		{			
			PageTable* identityPageTable = (PageTable*)PhysicalMemoryManager::AllocBlock();
			if (identityPageTable == NULL)
			{
				return false;
			}

			memset(identityPageTable, 0, sizeof(PageTable));
			
			for (int j = 0; j < PAGES_PER_TABLE; j++, frame += PAGE_SIZE, virt += PAGE_SIZE)
			{
				PTE page = 0;
				PageTableEntry::AddAttribute(&page, I86_PTE_PRESENT | I86_PTE_WRITABLE | I86_PTE_CPU_GLOBAL);
				PageTableEntry::SetFrame(&page, frame);				
				identityPageTable->m_entries[PAGE_TABLE_INDEX(virt)] = page;
			}

			//페이지 디렉토리에 페이지 디렉토리 엔트리(PDE)를 한 개 세트한다
			//0번째 인덱스에 PDE를 세트한다(가상주소가 0X00000000일시 참조됨)
			//앞에서 생성한 아이덴티티 페이지 테이블을 세트한다
			//가상주소 = 물리주소
			PDE* identityEntry = &_kernel_directory->m_entries[PAGE_DIRECTORY_INDEX((virt - 0x00400000))];
			PageDirectoryEntry::AddAttribute(identityEntry, I86_PDE_PRESENT | I86_PDE_WRITABLE);			
			PageDirectoryEntry::SetFrame(identityEntry, (uint32_t)identityPageTable);		
		}		

		SetCurPageDirectory(_kernel_directory);

		//페이지 디렉토리를 PDBR 레지스터에 로드한다		
		SetPageDirectory((UINT32)_kernel_directory);				
		
		return true;
	}	

	PageDirectory* GetKernelPageDirectory()
	{
		return _kernel_directory;
	}

	bool SetCurPageDirectory(PageDirectory* dir)
	{
		if (dir == NULL)
			return false;

		_cur_directory = dir;		

		return true;
	}

	PageDirectory* GetCurPageDirectory()
	{
		return _cur_directory;
	}

	bool MapDMAAddress(PageDirectory* pd, uintptr_t va, uintptr_t pa, uintptr_t end)
	{		
		for (int i = 0; va <= end; va += 0x1000, pa += 0x1000, i++)
		{
			MapPhysicalAddressToVirtualAddresss(pd, (uint32_t)va, (uint32_t)pa, I86_PTE_PRESENT | I86_PTE_WRITABLE | I86_PTE_CPU_GLOBAL);
		}

		return true;
	}

	void Dump()
	{
		
	}

	int MapAddress(PageDirectory* pd, DWORD startAddress, DWORD pageCount)
	{
		void* pAllocatedMemory = PhysicalMemoryManager::AllocBlocks(pageCount);

		if (pAllocatedMemory == nullptr)
		{
			LOG_FATAL("MapAddress Fail!!\n");
		}

		int endAddress = (uint32_t)startAddress + pageCount * PMM_BLOCK_SIZE;

		for (int i = 0; i < pageCount; i++)
		{
			uint32_t virt = (uint32_t)startAddress + i * PAGE_SIZE;
			uint32_t phys = (uint32_t)pAllocatedMemory + i * PAGE_SIZE;

			MapPhysicalAddressToVirtualAddresss(pd, virt, phys, I86_PTE_PRESENT | I86_PTE_WRITABLE | I86_PTE_CPU_GLOBAL);
		}

		return (int)pAllocatedMemory;
	}

	bool UnmapAddress(PageDirectory* pd, DWORD startAddress, DWORD pageCount)
	{
	
		for (int i = 0; i < pageCount; i++)
		{
			uint32_t virt = (uint32_t)startAddress + i * PAGE_SIZE;
			
			UnmapPhysicalAddress(pd, virt);
		}

		return true;
	}
}