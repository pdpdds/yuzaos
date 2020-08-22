#pragma once
#include <memory.h>
#include "AVLTree.h"
#include "Mutex.h"
#include "ktypes.h"

class Area;
class PageCache;
class PhysicalMap;
class Team;

// AddressSpace는 하드웨어 독립적인 가상 주소 공간을 표현한다.
// 즉 프로세스당 하나의 AddressSpace 객체를 가진다.
class AddressSpace 
{
public:
	AddressSpace();
	~AddressSpace();

	// 주소공간에서 인접한 범위의 가상주소 공간을 할당한다.
	// @param name 해당 Area의 이름. 디버깅 목적으로 사용
	// @param areaSize Area의 바이트 크기. 물리 페이지의 배수여야 한다.
	// @param wiring 페이지들이 상황에 따라 스왑가능한지를 결정한다. 이 값이 true이면 메모리에서 스왑되지 않는다.
	// @param protection 페이지가 슈퍼바이저, 유저에 의해 읽기/쓰기가 가능한지를 결정한다.
	// @param PageCache 이 Area에 매핑될 페이지를 얻을 수 있다. 이 값이 0이면 최초 Area에 접근시 항상 페이지 폴트가 발생할 것이다.
	// @param cacheOffset 생성될 Area의 선두 가상 주소를 위한 페이지 캐쉬 내에서의 오프셋. PageCache가 0이면 의미가 없다.
	// @param va : 이 Area의 가장 낮은 가상 주소. 물리 페이지의 배수에 해당해야 한다.
	// 이 값이 INVALID_PAGE라면 이 함수는 areaSize 크기의 사용되지 않은 주소공간을 찾아서 생성한 Area이 베이스 주소로 결정한다.
	// @param flags Area를 생성하기 위한 방법에 대한 플래그(최상위 주소, 또는 제일 낮은 주소에서 부터 유효한 주소공간을 찾을 지 등등)
	// @returns
	// - 성공시 Area 객체의 포인터를 반환한다.
	// - Area 객체 실패시 NULL을 반환한다.
	Area* CreateArea(const char name[], unsigned int areaSize, AreaWiring wiring, PageProtection protection,
		PageCache *cache, off_t cacheOffset, unsigned int va = INVALID_PAGE, int flags = 0);

	// 물리 메모리에 매핑하는 Area를 생성한다.
	// 디바이스는 물리 메모리에 매핑되므로 이 메소드는 디바이스 드라이버가 주로 사용한다.
	// @returns
	// - 성공시 Area 객체의 포인터를 반환한다.
	// - Area 객체 실패시 NULL을 반환한다.
	Area* MapPhysicalMemory(const char name[], unsigned int pa, unsigned int size,
		PageProtection protection, unsigned int fixed_va = INVALID_PAGE);

	// 기존 Area의 크기를 변경. 기존 Area와 관련된 Backing Store를 할당하거나 해제함.
	// @param area 재조정될 Area
	// @param newSize 재조정될 Area의 새로운 크기(바이트 단위). 물리 페이지의 배수여야 한다.
	// @returns
	// - E_NO_ERROR 성공적으로 재조정되었을 경우
	// - E_NO_MEMORY 
	int ResizeArea(Area *area, unsigned int newSize);

	// 이 주소 공간에서 해당 Area를 제거하고 이 Area와 관계했던 모든 페이지의 매핑을 해제한다
	void DeleteArea(Area* pArea);

	// 스레드가 이 주소공간에 매핑된 물리 페이지를 가지지 않은 메모리 영역 접근 시도를 할때 호출된다.
	// 페이지 폴트가 발생하면 해당 주소에 적당한 페이지를 매핑할 것이다.
	// @param va 유저가 접근 시도한 가상 주소
	// @param write 주소공간에 쓰기를 시도했으면 true, 읽기시도를 했으면 false다.
	// @param user 유저 코드가 이 주소에 접근 시도했으면 true, 슈퍼바이저 코드가 접근했으면 false
	// @returns
	//  - E_NOT_ALLOWED 스레드가 주어진 Area의 접근해서 실행할 권한을 가지고 있지 않다.
	// (예를들어 유저 스레드가 슈퍼바이저에만 허락된 읽기 전용의 Area에 쓰기 시도)
	// - E_IO Backing Store로 부터 데이터를 읽는도중 에러가 발생했다.
	// - E_BAD_ADDRESS 접근된 주소에 매핑할 수 있는 Area가 없다.
	// - E_NO_ERROR 페이지가 이 주소에 성공적으로 매핑됨
	int HandleFault(unsigned int va, bool write, bool user);

	// 이 주소 공간에서 최근에 접근한 적이 없는 페이지를 매핑해제한다.
	// 당연한 이야기지만 페이지를 회수하면 그만큼 여유분의 물리공간을 확보한다는 의미이므로 다른 프로세스에 넉넉히 자원을 제공해 줄 수 있다.
	void TrimWorkingSet();

	// 해당 주소공간에 대한 디버그 정보를 출력
	void Print() const;

	/// 아키텍처 의존적인 매핑 오브젝트를 얻는다.
	const PhysicalMap* GetPhysicalMap() const;


	/// Get the address space that the current thread is executing in
	static AddressSpace* GetCurrentAddressSpace();

	/// Get the address space for the kernel (note, although the kernel is physically mapped into
	/// the top of every address space, it has its own AddressSpace object representing it).
	static AddressSpace* GetKernelAddressSpace();

	/// Called at boot time to initialize structures
	static void Bootstrap();

	/// This thread just attempts to remove least frequently pages from address spaces
	/// @bug Shouldn't this be private?
	/// @bug Shouldn't this be NORETURN?
	static void PageDaemonLoop();
	
private:
	AddressSpace(PhysicalMap* physicalMap);
	//주어진 크기의 빈공간이 주소공간내에 존재하는지 확인하고
	//존재하면 해당 시작 가상주소값을, 존재하지 않으면 INVALID_PAGE를 반환한다.
	unsigned int FindFreeRange(unsigned int size, int flags = 0) const; 
	
	PhysicalMap* m_pPhysicalMap;
	RWLock m_areaLock;
	int m_faultCount; //페이지 폴트 카운트 수
	AVLTree fAreas;
	volatile int m_changeCount; //Area가 생성되거나 삭제 또는 리사이즈될때 카운트됨
	
	int fWorkingSetSize;
	int fMinWorkingSet;
	int fMaxWorkingSet;
	bigtime_t fLastWorkingSetAdjust;
	unsigned int fNextTrimAddress;


	static AddressSpace *fKernelAddressSpace;
	static void TrimTeamWorkingSet(void*, Team*);
};
