#include "stringdef.h"
#include <ctype.h>
#include "stdio.h"
#include <systemcall_impl.h>
#include <yuzaapi.h>

#include "commands.h"
#include <FileService.h>
#include <stat_def.h>
#include <unistd.h>

DOS_Drive* Drives[STORAGE_DEVICE_MAX] = { 0, };
extern int g_currentDriveIndex = 2;

long CmdCls(char *szCommand)
{
	return false;
}

long CmdKill(char *szCommand)
{
	/*int id = atoi(szCommand);

	kEnterCriticalSection();

	Process* pProcess = ProcessManager::GetInstance()->FindProcess(id);

	kLeaveCriticalSection();

	if (pProcess != nullptr)
	{
		printf("kill process : %s, ID : %d\n", pProcess->m_processName, pProcess->GetProcessId());
		ProcessManager::GetInstance()->RemoveProcess(pProcess->GetProcessId());
	}
	else
		printf("process don't exist(%d)\n", id);	*/

	return false;
}

long CmdProcessList(char *szCommand)
{
	/*kEnterCriticalSection();
	printf(" ID : Process Name\n");

	ProcessManager::ProcessList* processlist = ProcessManager::GetInstance()->GetProcessList();
	map<int, Process*>::iterator iter = processlist->begin();

	for (; iter != processlist->end(); ++iter)
	{
		Process* pProcess = (*iter).second;
		printf("  %d %s\n", pProcess->GetProcessId(), pProcess->m_processName);
	}

	kLeaveCriticalSection();*/

	return true;
}

long cmdMemState(char *szCommand)
{
	//SystemProfiler::GetInstance()->PrintMemoryState();
	return false;
}

long cmdCreateWatchdogTask(char* pName)
{
	/*kEnterCriticalSection();
	
	Process* pProcess = ProcessManager::GetInstance()->CreateProcessFromMemory("WatchDog", WatchDogProc, NULL, PROCESS_KERNEL);	
	kLeaveCriticalSection();
	
	if(pProcess == nullptr)
		printf("Can't create process\n");	*/

	return false;
}

long cmdTaskCount(char *szCommand)
{
	/*kEnterCriticalSection();

	ProcessManager::TaskList* taskList = ProcessManager::GetInstance()->GetTaskList();
	printf("current task count %d\n", taskList->size());

	kLeaveCriticalSection();*/
	return false;
}

long cmdGlobalState(char *szCommand)
{
	//SystemProfiler::GetInstance()->PrintGlobalState();
	return false;
}

long CmdExec(char *szCommand)
{

	int result = Syscall_CreateProcess(szCommand, nullptr, 16);
		
	if(result == 0)
	printf("Can't create process %s\n", szCommand);
	
	return false;
}

long cmdJpeg(char *szCommand)
{
	/*if (false == SkyGUISystem::GetInstance()->GUIEnable())
	{
		printf("GUI console mode only support this command.\n", szCommand);
		return false;
	}*/

	return false;
}

long cmdGUI(char *szCommand)
{
	//RequesGUIResolution();

	return false;
}

long cmdDebug(char* szCommand)
{
	Syscall_SysCallTest(szCommand);

	return false;
}

char* rtrimslash(char* str) {
	char* p;
	p = strchr(str, '\0');
	while (--p >= str && (*p == '/' || *p == '\\')) {};
	p[1] = '\0';
	return str;
}

long cmdCD(char* dir)
{
	char newdir[CROSS_LEN];

	DOS_Drive* pCurrentDrive = Drives[g_currentDriveIndex];

	if (pCurrentDrive == 0)
	{
		printf("internal error\n");
		return 0;
	}

	strcpy(newdir, pCurrentDrive->curdir);
	if (strcmp("..", dir) != 0)
	{
		//드라이브 변경
		if (strlen(dir) == 2 && dir[1] == ':')
		{
			char driveIndex = toupper(dir[0]);
			driveIndex -= 'A';
			

			if (driveIndex < 0 || driveIndex >= STORAGE_DEVICE_MAX)
			{
				printf("invalid drive\n");
				return 0;
			}

			bool result = Syscall_SetCurrentDriveId(driveIndex + 'A');

			if (result == true)
			{
				DOS_Drive* drive = Drives[driveIndex];

				if (drive == 0)
				{
					drive = new DOS_Drive();
					strcpy(drive->curdir, "\\");
					Drives[driveIndex] = drive;
					
				}
				g_currentDriveIndex = driveIndex;
			}
			else
			{
				printf("drive not exist\n");
			}
			return 0;
		}
		

		strcat(newdir, dir);

	}
	else
	{
		rtrimslash(newdir);
		char* ptr = (char*)strrchr(newdir, '\\');

		if (ptr)
		{
			*(ptr + 1) = 0;
		}
		else
		{
			if (newdir[0] == '\\')
				newdir[1] = 0;
		}
	}
	
	size_t len = strlen(newdir);
	if (len && (newdir[len - 1] != '\\'))
	{
		// It has to be a directory !
		struct stat test;
		if (stat(newdir, &test) != 0)
		{
			printf("cant't change directoy\n");
			return false;
		}
		if (test.st_mode != 0) 
		{
			printf("cant't change directoy\n");
			return false;
		}
	};

	long result = 1;
	if (strcmp("..", dir) != 0)
	{
		int temp = access(newdir, F_OK);
		result = (temp == 0);
	}
	
	if (result == 0)
		printf("cant't change directoy\n");
	else
	{
		
		strcpy(pCurrentDrive->curdir, newdir);
		if (strcmp("..", dir) != 0)
			strcat(pCurrentDrive->curdir, "\\");

		SetCurrentDirectory(pCurrentDrive->curdir);
	}

	return result;

}

void FillRect8(int x, int y, int w, int h, char col, int actualX, int actualY)
{
	char* lfb = (char*)0xF0000000;

	for (int k = 0; k < h; k++)
		for (int j = 0; j < w; j++)
		{
			int index = ((j + x) + (k + y) * actualX);
			lfb[index] = col;
			index++;
		}
}

long cmdSwitchGUI(char *szCommand)
{
	//성공적으로 그래픽 모드가 전환되는지 확인
	/*if(true == SwitchGUIMode(1024, 768, 261))
	{
		//그래픽 버퍼 주소를 매핑해야 한다.
		VirtualMemoryManager::MapDMAAddress(VirtualMemoryManager::GetCurPageDirectory(), 0xE0000000, 0xE0000000, 0xE0FF0000);
		VirtualMemoryManager::MapDMAAddress(VirtualMemoryManager::GetCurPageDirectory(), 0xF0000000, 0xF0000000, 0xF0FF0000);
		
		//사각형을 그린다.
		FillRect8(100, 100, 100, 100, 8, 1024, 768);
		for (;;);
	}*/
	
	return false;
}


long cmdPCI(char *szCommand)
{
	//DeviceDriverManager::GetInstance()->RequestPCIList();

	return false;
}

long cmdCallStack(char *szCommand)
{
	/*if (szCommand == nullptr)
	{
		Debugger::GetInstance()->TraceStackWithSymbol();
		return false;
	}

	int id = atoi(szCommand);
	Debugger::GetInstance()->TraceStackWithProcessId(id);*/

	return false;
}

long cmdDir(char *szCommand)
{
	DOS_Drive* pCurrentDrive = Drives[g_currentDriveIndex];

	if (pCurrentDrive == 0)
	{
		printf("internal error\n");
		return 0;
	}

	dir_information* pInfo = open_directory(pCurrentDrive->curdir);

	if (pInfo)
	{
		char entryName[CROSS_LEN];
		bool is_directory = false;
		bool result = read_directory_first(pInfo, entryName, is_directory);
		
		while (result)
		{
			printf("%s\n", entryName);
			result = read_directory_next(pInfo, entryName, is_directory);
		}
		
		close_directory(pInfo);
	}

	return false;
}