#pragma once
#include <windef.h>
#include <minwindef.h>

//백업 저장소는 물리 메모리가 아닌 곳에 데이터가 저장된 장소다.
//백업 저장소는 주소 공간의 IO 작업을 제공한다. 
//백업 저장소는 파일의 데이터를 나타내거나 스왑공간의 익명의 청크(덩어리)를 나타낸다.

class BackingStore 
{
public:
	virtual ~BackingStore();

	/// 백업 저장소의 특정 오프셋으로부터 읽을 수 있는 유효한 데이터가 있는지를 반환한다.
	/// 파일의 경우 이 메소드는 항상 true를 반환한다. SwapSpace의 경우 이 메소드는 주소에 기록할 수 없다면 false를 반환한다.
	/// false를 반환하면 가상 메모리는 물리 메모리 페이지를 0으로 채우고 디스크로부터 의미없는 데이터를 읽는 작업을 건너띈다.
	/// @param offset 이 백업 저장소의 바이트 오프셋
	/// @returns 읽을 수 있는 데이터가 있다면 true를 반환
	virtual bool HasPage(off_t offset) = 0;

	/// 특정 물리 디바이스로부터 메모리속으로 데이터를 읽어들인다.
	/// @param offset 이 백업 저장소 바이트 오프셋
	/// @param va 물리 페이지의 데이터를 복사할 가상 주소 위치(가상 주소는 이미 매핑되어 있다.)
	virtual int Read(off_t offset, void *va, int) = 0;

	/// 데이터를 메모리로부터 특정 물리 디바이스로 쓴다.
	/// @param offset 이 백업 저장소의 바이트 오프셋
	/// @param va 물리 페이지로 데이터를 복사하기 위한 가상주소 위치
	virtual int Write(off_t offset, const void *va) = 0;

	/// 특정한 양의 데이터가 백업 저장소에 기록될 수 있음을 보장
	/// @returns 실제 이용가능한 바이트 수를 반환
	virtual off_t Commit(off_t size) = 0;
};

inline BackingStore::~BackingStore()
{
}
