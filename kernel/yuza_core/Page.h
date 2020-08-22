#pragma once
#include <memory.h>
#include "Mutex.h"
#include <ds/queue.h>

// 물리 페이지 프레임에 대한 아키텍처 의존적인 추상화
class Page : public _QueueNode 
{
public:
	// 특정 물리 주소에 해당하는 페이지를 얻는다. 그리고 다른 스레드에 의해 스왑되지 않도록 마킹한다.
	// 락을 사용해서 블러킹을 할 필요는 없고 페이지 상태를 변경해서 다른 스레드가 이 페이지를 내버려두게 한다.
	static Page* LockPage(unsigned int pa);

	/// 이 페이지의 바이트단위 물리 주소를 얻어낸다.
	unsigned int GetPhysicalAddress() const;

	/// 사용하지 않은 페이지를 할당한다. 	
	/// @param clear 이 값이 true이면 페이지는 0으로 초기화 된다.
	static Page* Alloc(bool clear = false);

	/// 이 페이지를 사용가능한 리스트로 이동한다.
	void Free();

	/// 이 페이지가 작업중이라고 마킹(IO가 현재 수행중)
	void SetBusy();

	/// 이 페이지의 작업이 완료되었다고 마킹(IO가 끝남)
	void SetNotBusy();

	/// @returns 이 페이지로 작업중에 있다면 true를 반환
	inline bool IsBusy() const;

	// 페이지가 스왑되지 않도록 잠근다.
	void Wire();

	// 상황에 따라 스왑되도록 잠금을 해제한다. 정확히는 상태만 변경한다.
	void Unwire();

	/// 이용 가능한 페이지 전체 수를 얻는다.
	static int CountFreePages();

	// 바이트 크기로 전체 메모리 사이즈를 얻는다.
	static inline unsigned int GetMemSize();

	// 자료구조를 초기화하기 위해 부트 시간에 호출
	static void Bootstrap();

	/// 페이지를 초기화하는 낮은 우선순위의 스레드를 시작
	static void StartPageEraser();

	// 특정 물리 주소를 사용한 것으로 마크한다.
	// 부트로더나 힙에 의해 미리 할당된 페이지들을 초기화하는 동안 마킹하기 위해 사용된다.
	static void MarkUsed(unsigned int pa);

private:
	enum PageState {
		kPageFree,
		kPageTransition,
		kPageActive,
		kPageWired,
		kPageClear
	};

	void MoveToQueue(PageState);
	static int PageEraser(void*);
	static void PrintStats(int, const char**);

	Page *fHashNext;
	class PageCache *fCache;
	off_t fCacheOffset;
	Page *fCacheNext;
	Page **fCachePrev;
	volatile PageState m_state;

	static class Semaphore s_freePagesAvailable;
	static Page *fPages;
	static _Queue s_freeQueue;
	static _Queue fActiveQueue;
	static _Queue fClearQueue;
	static int s_pageCount;
	static int s_freeCount;
	static int fTransitionCount;
	static int fActiveCount;
	static int fWiredCount;
	static int fClearCount;

	// Page erase statistics
	static int64 fPagesCleared;
	static int64 fPagesRequested;
	static int64 fClearPagesRequested;
	static int64 fClearPageHits;
	static int64 fClearPagesUsedAsFree;


	friend class PageCache;
};

inline unsigned int Page::GetMemSize()
{
	return s_pageCount * PAGE_SIZE;
}

inline bool Page::IsBusy() const
{
	return m_state == kPageTransition;
}
