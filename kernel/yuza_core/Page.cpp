#include "BootParams.h"
#include <string>
#include "cpu_asm.h"
#include "Page.h"
#include "PhysicalMap.h"
#include "Semaphore.h"
#include <stringdef.h>
#include "SystemAPI.h"
#include "Thread.h"
#include "intrinsic.h"
#include <Debugger.h>
#include <BuildOption.h>
#include <memory_layout.h>
#include <kmalloc.h>

extern BootParams g_bootParams;
unsigned int* virtualMemory;

Semaphore Page::s_freePagesAvailable("Free Pages Available", 0);
Page* Page::fPages = 0;
_Queue Page::s_freeQueue;
_Queue Page::fActiveQueue;
_Queue Page::fClearQueue;
int Page::s_pageCount = 0;
int Page::s_freeCount = 0;
int Page::fTransitionCount = 0;
int Page::fActiveCount = 0;
int Page::fWiredCount = 0;
int Page::fClearCount = 0;

int64 Page::fPagesCleared = 0;
int64 Page::fPagesRequested = 0;
int64 Page::fClearPagesRequested = 0;
int64 Page::fClearPageHits = 0;
int64 Page::fClearPagesUsedAsFree = 0;

Page* Page::LockPage(unsigned int pa)
{
	Page *page = 0;
	int fl = DisableInterrupts();
	
	for (;;) 
	{
#if SKY_EMULATOR
		pa = pa - (unsigned int)virtualMemory;
#endif
		page = &fPages[pa / PAGE_SIZE];
		if (page->m_state != kPageTransition)
			break;

		RestoreInterrupts(fl);
		kSleep(20);
		fl = DisableInterrupts();
	}

	page->MoveToQueue(kPageTransition);
	RestoreInterrupts(fl);

	return page;
}
extern BootParams g_bootParams;

unsigned int Page::GetPhysicalAddress() const
{
#if SKY_EMULATOR
	int pageCount = (this - fPages);
	return  pageCount  * PAGE_SIZE + (unsigned int)virtualMemory;
#endif
	return (this - fPages) * PAGE_SIZE;	
}

Page* Page::Alloc(bool clear)
{
	Page *page = 0;

	s_freePagesAvailable.Wait();

	// Note that we grab pages from the tail of these queues.  This helps
	// processor cache utilization, but also improves performance of the
	// physical map page locking area.
	int fl = DisableInterrupts();
	fPagesRequested++;
	if (clear && fClearCount > 0) 
	{
		// There is already a pre-cleared page, use that.
		fClearPagesRequested++;
		fClearPageHits++;
		page = static_cast<Page*>(fClearQueue.GetTail());
	} else if (clear) 
	{
		// There aren't pre-cleared pages available, clear one now.
		fClearPagesRequested++;
		page = static_cast<Page*>(s_freeQueue.GetTail());

		char *va = PhysicalMap::LockPhysicalPage(page->GetPhysicalAddress());

		ClearPage(va);

		PhysicalMap::UnlockPhysicalPage(va);
		
	} else if (s_freeCount > 0) 
	{
		// This page should not be cleared, so just grab it off the
		// free queue.
		page = static_cast<Page*>(s_freeQueue.GetTail());
	} 
	else 
	{
		// This page should not be cleared, but there aren't any free
		// pages.  Use a cleared page instead.
		fClearPagesUsedAsFree++;
		page = static_cast<Page*>(fClearQueue.GetTail());
	}
	
	page->MoveToQueue(kPageTransition);
	RestoreInterrupts(fl);
	return page;
}

void Page::Free()
{
	ASSERT(m_state != kPageFree);
	ASSERT(fCache == 0);
	MoveToQueue(kPageFree);
}

void Page::SetBusy()
{
	MoveToQueue(kPageTransition);
}

void Page::SetNotBusy()
{
	ASSERT(fCache != 0);
	MoveToQueue(kPageActive);
}

void Page::Wire()
{
	MoveToQueue(kPageWired);
}

void Page::Unwire()
{
	ASSERT(fCache != 0);
	MoveToQueue(kPageActive);
}

int Page::CountFreePages()
{
	return s_freeCount;
}

void Page::Bootstrap()
{
#if SKY_EMULATOR
	virtualMemory = (unsigned int*)kmalloc_aligned(g_bootParams._memoryInfo._kHeapSize / 2, PAGE_SIZE);
	s_pageCount = (unsigned int)(g_bootParams._memoryInfo._kHeapSize / 2 / PAGE_SIZE);
	//fPages = (Page*)kmalloc_aligned(sizeof(Page) * s_pageCount, PAGE_SIZE);
	fPages = new Page[s_pageCount];
#else
	s_pageCount = g_bootParams._memoryInfo._memorySize / PAGE_SIZE;
	fPages = new Page[s_pageCount];
#endif
	
	
	s_freeCount = s_pageCount;
	s_freePagesAvailable.Release(s_pageCount, false);
	for (int pageIndex = 0; pageIndex < s_pageCount; pageIndex++)
	{
		fPages[pageIndex].fCache = 0;
		fPages[pageIndex].m_state = kPageFree;
		s_freeQueue.Enqueue(&fPages[pageIndex]);
	}
	
	Debugger::GetInstance()->AddCommand("pgstat", "Page statistics", PrintStats);
}

void Page::StartPageEraser()
{
	kCreateThread(PageEraser, "Page Eraser", 0, 1, 0);
}

//커널 로더가 할당한 물리 메모리들은 이미 사용중에 있다고
//마킹해야 한다.
void Page::MarkUsed(unsigned int pa)
{
	Page *page = &fPages[pa / PAGE_SIZE];
	if (page->m_state == kPageFree) 
	{
		// This never blocks (which would be fatal right now), as there
		// are enough pages available at this stage of the bootstrap; it
		// is just acquired to update the count.
		s_freePagesAvailable.Wait();
		page->MoveToQueue(kPageWired);
	} 
	//else
		//kprintf("Page %08x is in state %d\n", pa, page->m_state);

}

void Page::MoveToQueue(PageState newState)
{
	int fl = DisableInterrupts();
	switch (m_state) {
		case kPageFree:
			s_freeCount--;
			RemoveFromList();
			break;

		case kPageTransition:
			fTransitionCount--;
			break;

		case kPageActive:
			fActiveCount--;
			RemoveFromList();
			break;

		case kPageWired:
			fWiredCount--;
			break;

		case kPageClear:
			fClearCount--;
			RemoveFromList();
			break;

		default:
			kPanic("Page::MoveToQueue: bad page state 1");
	}

	m_state = newState;

	switch (m_state) {
		case kPageFree:
			ASSERT(fCache == 0);
			s_freeCount++;
			s_freeQueue.Enqueue(this);
			s_freePagesAvailable.Release(1, false);
			break;

		case kPageTransition:
			fTransitionCount++;
			break;

		case kPageActive:
			ASSERT(fCache != 0);
			fActiveCount++;
			fActiveQueue.Enqueue(this);
			break;

		case kPageWired:
			fWiredCount++;
			break;

		case kPageClear:
			fClearCount++;
			fClearQueue.Enqueue(this);
			s_freePagesAvailable.Release(1, false);
			break;

		default:
			kPanic("Page::MoveToQueue bad page state 2");
	}

	RestoreInterrupts(fl);
}

int Page::PageEraser(void*)
{
	for (;;) 
	{
		DisableInterrupts();
		s_freePagesAvailable.Wait();	// Shouldn't block, just update count

		Page *page = static_cast<Page*>(s_freeQueue.GetTail());
		if (!page) 
		{
			EnableInterrupts();
			s_freePagesAvailable.Release(1, false);
			kSleep(100);

			continue;
		}

		page->MoveToQueue(kPageTransition);
		EnableInterrupts();

		char *va = PhysicalMap::LockPhysicalPage(page->GetPhysicalAddress());
		ClearPage(va);
		PhysicalMap::UnlockPhysicalPage(va);
		
		DisableInterrupts();
		page->MoveToQueue(kPageClear);
		fPagesCleared++;
		EnableInterrupts();
	}

	return 0;
}

void Page::PrintStats(int, const char**)
{
	kprintf("Page Statistics\n");
	kprintf("  Free:        %5d (%2u)  %dk\n", s_freeCount, s_freeCount * 100 / s_pageCount, s_freeCount * PAGE_SIZE / 1024);
	kprintf("  Active:      %5d (%2u)  %dk\n", fActiveCount, fActiveCount * 100 / s_pageCount, fActiveCount * PAGE_SIZE / 1024);
	kprintf("  Wired:       %5d (%2u)  %dk\n", fWiredCount, fWiredCount * 100 / s_pageCount, fWiredCount * PAGE_SIZE / 1024);
	kprintf("  Transition:  %5d (%2u)  %dk\n", fTransitionCount, fTransitionCount * 100 / s_pageCount, fTransitionCount * PAGE_SIZE / 1024);
	kprintf("  Clear:       %5d (%2u)  %dk\n", fClearCount, fClearCount * 100 / s_pageCount, fClearCount * PAGE_SIZE / 1024);
	kprintf("  Total:       %5d        %2dk\n", s_pageCount, s_pageCount * PAGE_SIZE / 1024);
	kprintf("\n");
	kprintf("Pages requested:          %Ld\n", fPagesRequested);
	kprintf("Clear pages requested:    %Ld (%Ld)\n", fClearPagesRequested,
		fClearPagesRequested * 100 / fPagesRequested);
	kprintf("Pages cleared:            %Ld\n", fPagesCleared);
	kprintf("Clear page hits:          %Ld/%Ld (%Ld)\n", fClearPageHits, fClearPagesRequested,
		fClearPageHits * 100 / fClearPagesRequested);
	kprintf("Clear pages used as free: %Ld/%Ld (%Ld)\n", fClearPagesUsedAsFree,
		fPagesRequested - fClearPagesRequested, fClearPagesUsedAsFree * 100
		/ (fPagesRequested - fClearPagesRequested));
}
