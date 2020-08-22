#pragma once

#include "AVLTree.h"
#include "Resource.h"
#include "PageCache.h"

// 영역(area)는 백업 저장소의 가상 주소 범위를 표현한다.
// 영역은 메모리 매핑 파일이거나 유저 스택이나 커널 힙같은 익명의 스왑된 공간일 수 있다.
// 또는 커널 스택같은 잠긴 범위의 페이지일 수 있다.(스왑되어서는 안되는)
class Area : public AVLNode, public Resource {
public:
	inline Area(const char name[], PageProtection = 0, PageCache* = 0, off_t = 0,
		AreaWiring lock = AREA_WIRED);
	virtual ~Area();

	// 이 영역의 가장 낮은 가상 주소를 반환한다.
	inline unsigned int GetBaseAddress() const;

	// 가장 낮은 가상 주소에서 가장 높은 주소까지의 바이트 크기를 반환한다.
	inline unsigned int GetSize() const;

	// 이 영역과 관련된 페이지 캐쉬를 반환한다. 페이지 캐쉬는 이 영역에서 발생하는 페이지 폴트를 해결하기 위해 사용된다.
	inline PageCache* GetPageCache() const;

	// 이 영역상의 모든 페이지에 대한 아키텍처 독립적인 프로텍션 플래그를 얻는다.Get architecture independent protection flags for all pages in this area (can
	// 페이지는 유저나 시스템, 또는 둘 모두가 쓰거나 읽을 수 있다.
	inline PageProtection GetProtection() const;

	//이 영역의 최초 가상 주소에 대한 페이지 캐쉬 오프셋
	inline off_t GetCacheOffset() const;

	/// 페이지가 스왑될 수 있는지를 결정한다.
	inline AreaWiring GetWiring() const;

private:
	PageProtection m_protection;
	PageCache *fPageCache;
	off_t fCacheOffset;
	AreaWiring m_wiring;
};

inline Area::Area(const char name[], PageProtection protection, PageCache *cache, off_t offset, AreaWiring lock)
	:	Resource(OBJ_AREA, name),
	m_protection(protection),
		fPageCache(cache),
		fCacheOffset(offset),
	m_wiring(lock)
{
	if (fPageCache)
		fPageCache->AcquireRef();

	AcquireRef();
}

inline Area::~Area()
{
	if (fPageCache)
		fPageCache->ReleaseRef();
}

inline unsigned int Area::GetBaseAddress() const
{
	return GetLowKey();
}

inline unsigned int Area::GetSize() const
{
	return GetHighKey() - GetLowKey() + 1;
}

inline PageCache* Area::GetPageCache() const
{
	return fPageCache;
}

inline PageProtection Area::GetProtection() const
{
	return m_protection;
}

inline off_t Area::GetCacheOffset() const
{
	return fCacheOffset;
}

inline AreaWiring Area::GetWiring() const
{
	return m_wiring;
}