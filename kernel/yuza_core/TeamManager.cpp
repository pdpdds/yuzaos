#include "TeamManager.h"
#include <Team.h>
#include <AddressSpace.h>
#include <Thread.h>
#include <Debugger.h>
#include <intrinsic.h>

TeamManager* TeamManager::m_pTeamManager = 0;

TeamManager::TeamManager()
	: m_teamId(0)
{

}

void TeamManager::Bootstrap()
{
	Team* init = CreateTeam("kernel", AddressSpace::GetKernelAddressSpace());
	
	Thread::GetRunningThread()->SetTeam(init);
	
	Debugger::GetInstance()->AddCommand("ps", "list running threads", PrintThreads);
	Debugger::GetInstance()->AddCommand("areas", "list user areas", PrintAreas);
	Debugger::GetInstance()->AddCommand("handles", "list handles", PrintHandles);
	Debugger::GetInstance()->AddCommand("ps2", "list loaded moudle address", PrintTeamModules);

	
}

Team* TeamManager::CreateTeam(const char* name)
{
	m_teamId++;
	Team* team = new Team(name, m_teamId);

	if (team == 0)
		return 0;

	m_teamList.AddToTail(team);
	return team;
}

Team* TeamManager::CreateTeam(const char* name, AddressSpace* addressSpace)
{
	m_teamId++;
	Team* team = new Team(name, addressSpace, m_teamId);
	
	m_teamList.AddToTail(team);
	return team;
}

void TeamManager::PrintAreas(int argc, const char** argv)
{
	List& teamList = TeamManager::GetInstance()->GetTeamList();

	for (const ListNode* node = teamList.GetHead(); node; node = teamList.GetNext(node))
	{
		const Team* team = static_cast<const Team*>(node);
		kprintf("Team %s\n", team->GetName());
		team->GetAddressSpace()->Print();
	}
}

void TeamManager::PrintHandles(int argc, const char** argv)
{
	List& teamList = TeamManager::GetInstance()->GetTeamList();

	for (const ListNode* node = teamList.GetHead(); node; node = teamList.GetNext(node))
	{
		const Team* team = static_cast<const Team*>(node);
		kprintf("Team %s\n", team->GetName());
		team->GetHandleTable()->Print();
	}
}

Team* TeamManager::FindTeam(int teamId)
{
	List& teamList = TeamManager::GetInstance()->GetTeamList();

	int fl = DisableInterrupts();
	Team* team = nullptr;
	for (ListNode* node = teamList.GetHead(); node; node = teamList.GetNext(node))
	{
		Team* tempTeam = static_cast<Team*>(node);

		if (teamId == tempTeam->m_teamId)
		{
			team = tempTeam;
			break;
		}
	}

	if (team)
		team->AcquireRef();

	RestoreInterrupts(fl);

	return team;
}

void TeamManager::PrintThreads(int argc, const char** argv)
{
	const char* kThreadStateName[] = { "Created", "Wait", "Ready", "Running", "Dead" };
	int threadCount = 0;
	int teamCount = 0;

	List& teamList = TeamManager::GetInstance()->GetTeamList();
	const ListNode* node = teamList.GetHead();

	for (; node != nullptr; node = teamList.GetNext(node))
	{
		const Team* team = static_cast<const Team*>(node);
		teamCount++;
		printf("Team %s\n", team->GetName());
		printf("Name                 State    CPRI BPRI\n");
		for (Thread* thread = team->m_pThreadList; thread; thread = thread->fTeamListNext)
		{
			threadCount++;
			printf("%20s %8s %4d %4d\n", thread->GetName(), kThreadStateName[thread->GetState()], thread->GetCurrentPriority(), thread->GetBasePriority());
		}

		kprintf("\n");
	}

	kprintf("%d Threads  %d Teams\n", threadCount, teamCount);
}

#include <ModuleManager.h>
#include <LoadDLL.h>

void TeamManager::PrintTeamModules(int argc, const char** argv)
{
	kprintf("%s\n", Thread::GetRunningThread()->GetTeam()->GetName());

	auto iter = Thread::GetRunningThread()->GetTeam()->m_loadedDllList.begin();

	for (; iter != Thread::GetRunningThread()->GetTeam()->m_loadedDllList.end(); iter++)
	{
		LOAD_DLL_INFO* pInfo = (LOAD_DLL_INFO*)(*iter);

		kprintf("%s 0x%x\n", pInfo->moduleName, pInfo->image_base);

	}


}

void TeamManager::DoForEach(void (*EachTeamFunc)(void*, Team*), void* cookie)
{
	List& teamList = TeamManager::GetInstance()->GetTeamList();
	int fl = DisableInterrupts();
	Team* team = static_cast<Team*>(teamList.GetHead());
	team->AcquireRef();

	while (team)
	{
		RestoreInterrupts(fl);
		EachTeamFunc(cookie, team);
		DisableInterrupts();
		Team* next = static_cast<Team*>(teamList.GetNext(team));
		if (next)
			next->AcquireRef();

		team->ReleaseRef();
		team = next;
	}

	RestoreInterrupts(fl);
}

