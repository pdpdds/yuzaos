#pragma once

#include "Mutex.h"
#include "ktypes.h"
#include "memory.h"

const int kLevel1Size = 1024;
const int kLevel2Size = 1024;
const int kSerialNumberCount = 2048;

class Resource;

// HandleTable은 리소스 포인터 매핑 오브젝트에 대한 식별자다. 
// HandleTable은 2단계 테이블로 구현된다.
class HandleTable {
public:
	HandleTable();
	~HandleTable();

	// 리소스를 테이블에 등록한다. 그리고 이 리소스를 위한 식별자를 반환한다.
	int Open(Resource*);

	// 테이블에서 주어진 리소스를 제거한다.
	// @param id Open 함수가 반환했던 리소스에 대한 식별자
	void Close(int id);

	// 테이블에서 주어진 id로 리소스를 찾는다.
	Resource* GetResource(int id, int matchType = OBJ_ANY) const;

	// 이 테이블에 있는 모든 핸들을 출력한다.
	void Print() const;

private:
	struct SubTable *fMainTable[kLevel1Size];
	mutable RWLock fLock;
	int fFreeHint;
};
