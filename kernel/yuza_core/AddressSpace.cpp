#include <string>
#include "AddressSpace.h"
#include "Area.h"
#include "cpu_asm.h"
#include "memory_layout.h"
#include "Page.h"
#include "PageCache.h"
#include "PhysicalMap.h"
#include <stringdef.h>
#include "Team.h"
#include "Thread.h"
#include "math.h"
#include "SystemAPI.h"
#include "intrinsic.h"
#include <StackTracer.h>
#include <TeamManager.h>

#if SKY_EMULATOR
extern unsigned int kHeapBase;
extern unsigned int kIOAreaBase;
#endif 

const int kDefaultWorkingSet = 0x40000;
const int kDefaultMinWorkingSet = 0x10000;
const int kDefaultMaxWorkingSet = 0x200000;
const int kWorkingSetAdjustInterval = 2000000;
const int kMinFaultsPerSecond = 20;
const int kMaxFaultsPerSecond = 100;
const int kMinFreePages = 40;
const int kWorkingSetIncrement = PAGE_SIZE * 10;
const int kTrimInterval = 500000;

AddressSpace* AddressSpace::fKernelAddressSpace = 0;

AddressSpace::AddressSpace()
	: m_pPhysicalMap(new PhysicalMap),
	m_changeCount(0),
		fWorkingSetSize(kDefaultWorkingSet),
		fMinWorkingSet(kDefaultMinWorkingSet),
		fMaxWorkingSet(kDefaultMaxWorkingSet),
	m_faultCount(0),
		fLastWorkingSetAdjust(SystemTime()),
		fNextTrimAddress(0)
{
	//���ο� ���� �ּ� ������ ������ �� ȣ��ȴ�.
	//�ּ� 0���� ���� �̹��� �Ǵ� ��Ʈ�� ����Ʈ�� �ε�� kUserBase ���ϴ�
	//���� �������� �����ؼ� �� ������ ������ ������ �� �ֵ��� �Ѵ�.
	//�׸��� Ŀ�� ������ �����ؼ� �ش� �ּҰ������� �Ҵ��� �Ұ��ϵ��� �����Ѵ�.
	fAreas.Add(new Area("(null area)"), 0, kUserBase - 1);
	fAreas.Add(new Area("(kernel)"), kKernelBase, kKernelTop);
}

AddressSpace::~AddressSpace()
{
	for (;;) 
	{
		//�ּҰ����� �Ҹ��Ҷ� ��� Area�� Area �ʿ��� �����Ѵ�.
		//�ݺ��ڴ� ���� �����ؾ� �Ѵ�. �ֳ��ϸ� Ʈ���κ��� �� Area�� ���ŵ� �Ŀ��� ��ȿ���� �ʱ� �����̴�.
		AVLTreeIterator iterator(fAreas, true);
		Area *area = static_cast<Area*>(iterator.GetCurrent());
		if (area == 0)
			break;

		//���������� �������� �ؼ��� �ȵȴ�. �� �����δ�
		//1. �ּҰ����� ��·�� ����� ���̱� �����̴�.
		//2. �� �ּҰ����� Ŀ���� �ּҰ����� �� �ֱ� �����̴�.
		fAreas.Remove(area);
		area->ReleaseRef();
	}

	delete m_pPhysicalMap;
}

Area* AddressSpace::CreateArea(const char name[], unsigned int size, AreaWiring wiring,
	PageProtection protection, PageCache *cache, off_t offset, unsigned int va,
	int flags)
{
	if (size % PAGE_SIZE != 0 || size == 0)
		return 0;

	if (cache != 0 && cache->Commit(offset + size) != offset + size)
		return 0;

	m_areaLock.LockWrite();
	if (va == INVALID_PAGE)
		va = FindFreeRange(size, flags);
	else if (va % PAGE_SIZE != 0 || !fAreas.IsRangeFree(va, va + size - 1))
		va = INVALID_PAGE;
	
	Area *area = 0;
	if (va != INVALID_PAGE) 
	{
		area = new Area(name, protection, cache, offset, wiring);	

		if (wiring & AREA_WIRED) 
		{
			PageCache* cache = area->GetPageCache();
			for (unsigned int areaOffset = 0; areaOffset < size; areaOffset += PAGE_SIZE)
			{

				Page *page = cache->GetPage(area->GetCacheOffset() + areaOffset, false);

				ASSERT(page != nullptr);

				page->Wire();
		
				m_pPhysicalMap->Map(va + areaOffset, page->GetPhysicalAddress(), protection);

			}
		}

		fAreas.Add(area, va, va + size - 1);
	}	

	m_changeCount++;
	m_areaLock.UnlockWrite();
	return area;
}

Area* AddressSpace::MapPhysicalMemory(const char name[], unsigned int pa,
	unsigned int size, PageProtection protection, unsigned int fixed_va)
{
	unsigned int va = 0;
	m_areaLock.LockWrite();

	if (fixed_va != INVALID_PAGE)
	{
		va = fixed_va;
	}
	else
	{
		va = FindFreeRange(size);
	}
	
	if (va == INVALID_PAGE) 
	{
		m_areaLock.UnlockWrite();
		return 0;
	}

	Area *area = new Area(name, protection, 0, 0, AREA_WIRED);
	fAreas.Add(area, va, va + size - 1);
	for (unsigned int areaOffset = 0; areaOffset < size; areaOffset += PAGE_SIZE)
		m_pPhysicalMap->Map(va + areaOffset, pa + areaOffset, protection);

	m_changeCount++;
	m_areaLock.UnlockWrite();
	return area;
}

int AddressSpace::ResizeArea(Area *area, unsigned int newSize)
{
	if (newSize == 0)
		return E_INVALID_OPERATION;

	m_areaLock.LockWrite();
	if (newSize > area->GetSize()) 
	{
		// ũ�Ⱑ ���� ������ ���� ũ�⺸�� ũ�ٸ� ũ�⸦ ������Ų��.
		if (area->GetPageCache()->Commit(newSize) != newSize) {
			m_areaLock.UnlockWrite();
			return E_NO_MEMORY;
		}

		if (area->GetBaseAddress() + newSize < area->GetBaseAddress()	// wrap
			|| !fAreas.IsRangeFree(area->GetBaseAddress() + area->GetSize(),
			area->GetBaseAddress() + newSize))	{
			m_areaLock.UnlockWrite();
			return E_NO_MEMORY;
		}

//area�� �Ӽ��� AREA_WIRED��� �޸𸮿��� ���ҵǾ�� �ȵǹǷ�
//�������� �Ӽ��� kPageWired�� �����Ѵ�.
//AREA_WIRED�� �ƴ϶�� ���� ���� �����Ӱ� ������ �ʿ䰡 ����.
		if (area->GetWiring() == AREA_WIRED) 
		{
			for (unsigned int areaOffset = area->GetSize(); areaOffset < newSize; areaOffset += PAGE_SIZE)
			{
				Page *page = area->GetPageCache()->GetPage(area->GetCacheOffset() + areaOffset, false);
				page->Wire();
				m_pPhysicalMap->Map(area->GetBaseAddress() + areaOffset, page->GetPhysicalAddress(), area->GetProtection());
			}
		}
	} 
	else if (newSize < area->GetSize()) 
	{
		// ������ ��ҽ�Ų��.
		m_pPhysicalMap->Unmap(area->GetBaseAddress() + newSize, area->GetSize() - newSize);
		area->GetPageCache()->Commit(newSize);
	}

	fAreas.Resize(area, newSize);
	m_changeCount++;
	m_areaLock.UnlockWrite();
	return E_NO_ERROR;
}

void AddressSpace::DeleteArea(Area *area)
{
	m_areaLock.LockWrite();
	fAreas.Remove(area);
	m_pPhysicalMap->Unmap(area->GetBaseAddress(), area->GetSize());
	area->ReleaseRef();
	m_changeCount++;
	m_areaLock.UnlockWrite();
}

int AddressSpace::HandleFault(unsigned int va, bool write, bool user)
{
	va &= ~(PAGE_SIZE - 1); // ������ ��迡 �°� �����ּҸ� ������.
	
	m_areaLock.LockRead();
	int lastChangeCount = m_changeCount;
	Area *area = static_cast<Area*>(fAreas.Find(va));
	if (area == 0)
	{
		m_areaLock.UnlockRead();
		return E_BAD_ADDRESS;
	}

	PageProtection protection = area->GetProtection();

	kDebugPrint("handle fault retrived %x %x %d %d %d\n", area, va, user, write, protection);

	if ((user && write && !(protection & USER_WRITE))
		|| (user && !write && !(protection & USER_READ))
		|| (!user && write && !(protection & SYSTEM_WRITE))
		|| (!user && !write && !(protection & SYSTEM_READ))) 
	{
		m_areaLock.UnlockRead();
		return E_NOT_ALLOWED;
	}

	PageCache *cache = area->GetPageCache();

//�ý��� ������ ������ ĳ���� 0�ϼ��� ����.
	if (cache == 0) 
	{
		m_areaLock.UnlockRead();
		return E_NOT_ALLOWED;
	}

//������ ĳ���� ���纻�ΰ�?
//���� ��� Ư�� ���ϰ� �������� ���ε� ���� ������ ĳ���� ���纻����?
	bool copy = cache->IsCopy();
	cache->AcquireRef();
	m_areaLock.UnlockRead();

//�Ϲ������� ������ ĳ�� �������� 0�̴�.
//������ Area�� ���� �ּҴ� ���� ������ ĳ���� Ư�� ���� �����°� ���ε� ���� �ִ�.
	off_t offset = va - area->GetBaseAddress() + area->GetCacheOffset();
	Page *page = cache->GetPage(offset, write && cache->IsCopy());
	cache->ReleaseRef();

//������ ��Ʈ�� ����Ų ���� �ּҿ� �ش��ϴ� �������� �ݵ�� �߰��ؾ� �Ѵ�.
	if (page == 0)
		return E_IO;

	m_areaLock.LockRead();
	if (lastChangeCount != m_changeCount) 
	{
		//�۾� ���ൿ�� Area�� ������ �߻��ߴ�.
		//���� �ɾ ������ ��Ʈ �ڵ鷯 ó�� ��Ȳ���� Area�� ����Ǿ����� Ȯ���Ѵ�.
		//����Ǿ��ٸ� ó���� �����Ѵ�.
		//�����ּҰ� ���� Area, �� Area�� ������ ������ ĳ��, �׸��� ������ ĳ�� �������� �����ؾ� �Ѵ�.
		Area *newArea = static_cast<Area*>(fAreas.Find(va));
		if (newArea != area || newArea->GetPageCache() != cache ||
			newArea->GetCacheOffset() != offset)
			m_areaLock.UnlockRead();
			return E_BAD_ADDRESS;
	}
//ī�� �� ����Ʈ(COW) ���������� �б⿴�ٸ� �� �������� ���� ������ ĳ����
//�����ǹǷ� ���� �����ӿ� ���� ���� �ʵ��� �Ӽ��� ��������.
	if (copy && !write)
		protection &= ~(USER_WRITE | SYSTEM_WRITE);

//�����ּҿ� ���� �޸𸮸� �����Ѵ�.
	m_pPhysicalMap->Map(va, page->GetPhysicalAddress(), protection);
	m_areaLock.UnlockRead();
	AtomicAdd(&m_faultCount, 1);
	return E_NO_ERROR;
}

void AddressSpace::TrimWorkingSet()
{
	int mappedMemory = m_pPhysicalMap->CountMappedPages() * PAGE_SIZE;
	bigtime_t now = SystemTime();

	// Adjust the working set size based on fault rate.
	if (now - fLastWorkingSetAdjust > kWorkingSetAdjustInterval) 
	{
		int fl = DisableInterrupts();
		int faultsPerSecond = static_cast<int64>(m_faultCount)
			* 1000000 / (now - fLastWorkingSetAdjust);
		if (faultsPerSecond > kMaxFaultsPerSecond
			&& mappedMemory >= fWorkingSetSize
			&& fWorkingSetSize < fMaxWorkingSet) 
		{
			fWorkingSetSize = MIN(fWorkingSetSize + kWorkingSetIncrement, fMaxWorkingSet);
		} 
		else if (faultsPerSecond < kMinFaultsPerSecond
			&& mappedMemory <= fWorkingSetSize
			&& fWorkingSetSize > fMaxWorkingSet
			&& Page::CountFreePages() < kMinFreePages) 
		{
			fWorkingSetSize = MAX(fWorkingSetSize - kWorkingSetIncrement, fMaxWorkingSet);
		}

		fLastWorkingSetAdjust = now;
		m_faultCount = 0;
		RestoreInterrupts(fl);
	}

	// Trim some pages if needed
	while (mappedMemory > fWorkingSetSize) 
	{

        break;

	}
}

const PhysicalMap* AddressSpace::GetPhysicalMap() const
{
	return m_pPhysicalMap;
}

AddressSpace* AddressSpace::GetCurrentAddressSpace()
{
	return Thread::GetRunningThread()->GetTeam()->GetAddressSpace();
}

AddressSpace* AddressSpace::GetKernelAddressSpace()
{
	return fKernelAddressSpace;
}

void AddressSpace::Bootstrap()
{
	fKernelAddressSpace = new AddressSpace(PhysicalMap::GetKernelPhysicalMap());
}

void AddressSpace::Print() const
{
	kprintf("Name                 Start    End      Cache    Protection\n");
	for (AVLTreeIterator iterator(fAreas, true); iterator.GetCurrent(); iterator.GoToNext()) 
	{
		const Area *area = static_cast<const Area*>(iterator.GetCurrent());
		PageCache *cache = area->GetPageCache();
		PageProtection prot = area->GetProtection();
		kprintf("%20s %08x %08x %p %c%c%c%c%c%c\n", area->GetName(),
			area->GetBaseAddress(),
			area->GetBaseAddress() + area->GetSize() - 1, cache,
			(prot & USER_READ) ? 'r' : '-',
			(prot & USER_WRITE) ? 'w' : '-',
			(prot & USER_EXEC) ? 'x' : '-',
			(prot & SYSTEM_READ) ? 'r' : '-',
			(prot & SYSTEM_WRITE) ? 'w' : '-',
			(prot & SYSTEM_EXEC) ? 'x' : '-');
	}

	kprintf("Resident: %dk Working Set: %dk Min: %dk Max: %dk Faults/Sec: %Ld\n",
		m_pPhysicalMap->CountMappedPages() * PAGE_SIZE / 1024, fWorkingSetSize / 1024, fMinWorkingSet / 1024,
		fMaxWorkingSet / 1024, static_cast<int64>(m_faultCount) * 1000000
		/ kWorkingSetAdjustInterval);
	kprintf("\n");
}

// Ŀ�� �ּ� ���� ������
AddressSpace::AddressSpace(PhysicalMap * physicalMap)
	: m_pPhysicalMap(physicalMap),
		fWorkingSetSize(kDefaultWorkingSet),
		fMinWorkingSet(kDefaultMinWorkingSet),
		fMaxWorkingSet(kDefaultMaxWorkingSet),
	m_faultCount(0),
		fLastWorkingSetAdjust(SystemTime()),
		fNextTrimAddress(0),
	m_changeCount(0)
{
	fAreas.Add(new Area("Identity Area", SYSTEM_READ | SYSTEM_EXEC), 0, PAGE_SIZE * 1024 * 4);  //16MB
	fAreas.Add(new Area("Kernel Text", SYSTEM_READ | SYSTEM_EXEC), kKernelBase, 0x1000000);
	fAreas.Add(new Area("Kernel Heap", SYSTEM_READ | SYSTEM_WRITE), kHeapBase, 0x04000000);
	fAreas.Add(new Area("Hyperspace", SYSTEM_READ | SYSTEM_WRITE), kIOAreaBase, kIOAreaTop);
	Area *kstack = new Area("Init Stack", SYSTEM_READ | SYSTEM_WRITE);
	fAreas.Add(kstack, kKernelStackTop - kKernelStackSize, kKernelStackTop - 1);
	Thread::GetRunningThread()->SetKernelStack(kstack);
}

unsigned int AddressSpace::FindFreeRange(unsigned int size, int flags) const
{
	unsigned int base = INVALID_PAGE;

	//�ּ� 0���� ���� �� ������ ã�´�.
	if (flags & SEARCH_FROM_BOTTOM) 
	{
		unsigned int low = 0;
		for (AVLTreeIterator iterator(fAreas, true); iterator.GetCurrent(); iterator.GoToNext()) 
		{
			const Area *area = static_cast<const Area*>(iterator.GetCurrent());
			if (area->GetBaseAddress() - low >= size) 
			{
				base = low;
				break;
			}

			low = area->GetHighKey() + 1;
		}
	} 
	else 
	{
		//4GB���� �Ʒ��� �˻��� �����鼭 ������� ã�´�.
		unsigned int high = kAddressSpaceTop;
		for (AVLTreeIterator iterator(fAreas, false); iterator.GetCurrent(); iterator.GoToNext()) 
		{
			const Area *area = static_cast<const Area*>(iterator.GetCurrent());

			if (high - area->GetHighKey() >= size) 
			{
				base = high + 1 - size;
				break;
			}

			high = area->GetBaseAddress() - 1;
		}
	}

	return base;
}

void AddressSpace::PageDaemonLoop()
{
	for (;;) 
	{
		kSleep(kTrimInterval);
		TeamManager::DoForEach(TrimTeamWorkingSet, 0);		
	}
}

void AddressSpace::TrimTeamWorkingSet(void* , Team *team)
{
	team->GetAddressSpace()->TrimWorkingSet();
}
