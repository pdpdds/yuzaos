#pragma once
#include "HandleTable.h"
#include "Resource.h"
#include <ThreadContext.h>
#include <kmalloc.h>
#include <list>

class AddressSpace;
class Thread;
class Image;

// Team 클래스는 스레드를 모아둔 그룹이며 프로세스와 동일한 개념이다.
// Team 클래스는 스레드 그룹과 주소공간을 가지고 있다.
class Team : public Resource, public ListNode 
{
	friend class TeamManager;
public:
	Team(const char* name, int teamId);
	virtual ~Team();
	inline AddressSpace* GetAddressSpace() const;

	// 스레드가 생성될때 호출된며 팀의 스레드 리스트에 생성된 스레드를 추가한다.
	void ThreadCreated(Thread*);

	// 스레드가 종료될때 호출된다. 스레드 리스트에서 해당 스레드를 제거한다.
	void ThreadTerminated(Thread*);

	// 각각의 팀은 자신만의 핸들 테이블을 소유한다.
	inline const HandleTable *GetHandleTable() const;
	inline HandleTable* GetHandleTable();

	bool CreateHeap();

	// 스레드 핸들로부터 스레드 객체를 얻는다.
	Thread* GetThread(HANDLE hThread);

	int GetTeamId() { return m_teamId; }

	int StartMainThread(ThreadParam* pParam);
	bool MapDLL(void* image);

	void SetTaskId(QWORD taskId) { m_taskId = taskId; };
	QWORD GetTaskId() { return m_taskId; }

	Semaphore* waitSemaphore;

	HANDLE GetUserHeap() { return m_heapHandle; }
	LPVOID AllocateMemory(DWORD dwSize);
	bool DeallocateMemory(LPVOID lpAddress);

	char m_szCWD[MAXPATH];
	char m_currentDrive;
	HANDLE m_mainThreadHandle;
	HANDLE m_moduleHandle;

private:
	Team(const char* name, AddressSpace* addressSpace, int teamId);
	
	Thread* m_pThreadList;	
	Image* m_image;
	HandleTable m_handleTable;
	AddressSpace* m_addressSpace;
	int m_teamId;
	QWORD m_taskId;
	HANDLE m_heapHandle;
	std::list<void*> m_loadedDllList;
	//heap_t* m_userHeap;
};

inline AddressSpace* Team::GetAddressSpace() const
{
	return m_addressSpace;
}

inline const HandleTable* Team::GetHandleTable() const
{
	return &m_handleTable;
}

inline HandleTable* Team::GetHandleTable()
{
	return &m_handleTable;
}