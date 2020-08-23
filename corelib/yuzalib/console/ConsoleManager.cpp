#include "ConsoleManager.h"
#include "stringdef.h"
#include "stdio.h"
#include "../command/commandTable.h"
#include "memory.h"
#include <systemcall_impl.h>

void showHelp(void)
{
	printf("Framework Version: %s\n", gFrameWorkVersion);
	printf("\n");

	for (ULONG i = 0; i < g_NumberOfCommands; i++)
	{
		if (g_Commands[i].szCommand[0] != '_')
		{
			printf(("%s : %s\n"), (const char*)(g_Commands[i].szCommand), (const char*)(g_Commands[i].comments));
		}
	}

	printf("\n");
}

long processCommandLine(const char *a_szCommand)
{
	ULONG   i;
	char   szCmdCopy[256];
	char   szDelim[] = " ";
	char  *pCurrentToken;

	if (0 == strlen(a_szCommand))
		return false;

	memset(szCmdCopy, 0, 256);

	bool result = false;
	for (i = 0; i < g_NumberOfCommands; i++)
	{
		if (g_Commands[i].bHasArguments)
		{
			// Copy the command.
			strcpy(szCmdCopy, a_szCommand);

			if (g_Commands[i].ProcessingFunc != nullptr)
			{
				// This returns the command sub-string.
				pCurrentToken = strtok(szCmdCopy, szDelim);

				if (0 == stricmp(pCurrentToken, g_Commands[i].szCommand))
				{
					pCurrentToken = strtok(NULL, szDelim);

					if (pCurrentToken != nullptr)
					{
						g_Commands[i].ProcessingFunc(pCurrentToken);
						return true;
					}
					else
					{
						printf("Argument insufficient\n");
						return false;
					}

				}
			}
		}
		else
		{
			// Copy the command.
			strcpy(szCmdCopy, a_szCommand);

			if (g_Commands[i].ProcessingFunc != nullptr)
			{
				// This returns the command sub-string.
				pCurrentToken = strtok(szCmdCopy, szDelim);

				if (0 == stricmp(pCurrentToken, g_Commands[i].szCommand))
				{
					pCurrentToken = strtok(NULL, szDelim);

					if (pCurrentToken != nullptr)
					{
						return g_Commands[i].ProcessingFunc(pCurrentToken);
					}
					else
					{
						g_Commands[i].ProcessingFunc(NULL);
						return false;
					}

				}
			}
		}
	}

	if (result == false)
	{
		if (0 == strcmp("help", a_szCommand))
		{
			showHelp();
			return true;
		}
	}

	if (result == false)
	{
		strcpy(szCmdCopy, a_szCommand);
		pCurrentToken = strtok(szCmdCopy, szDelim);
		
		if (pCurrentToken)
		{
			char* pArg = strtok(NULL, "\n");
			if(pArg)
				printf("token %s\n", pArg);

			char execPath[MAX_PATH];
			GetCurrentDirectory(MAX_PATH, execPath);
			
			strcat(execPath, pCurrentToken);
			//printf("execute file %s\n", execPath);

			int teamId = Syscall_CreateProcess(pCurrentToken, pArg, 16);

			if(teamId > 0)
				result = true;

			bool graphicMode = Syscall_IsGraphicMode();
			if (graphicMode == false)
			{
				if (teamId != 0)
					Syscall_WaitForChildProcess(teamId);
			}	
		}
	}

	return result;
}

ConsoleManager::ConsoleManager()
{
}

ConsoleManager::~ConsoleManager()
{
}


bool ConsoleManager::RunCommand(const char* buf)
{
	if (buf[0] == '\0')
	{
		return false;
	}

	if (strcmp(buf, "exit") == 0)
	{
		return true;	
	}

	if(false == processCommandLine(buf))
		printf("Command not found....\n");

	return false;
}

extern DOS_Drive* Drives[STORAGE_DEVICE_MAX];
extern int g_currentDriveIndex;

char* ConsoleManager::MakePathName()
{

	static char pathName[MAX_PATH];
	DOS_Drive* pDrive = Drives[g_currentDriveIndex];
	if (!pDrive)
	{
		pDrive = new DOS_Drive();
		Drives[g_currentDriveIndex] = pDrive;
		strcpy(Drives[g_currentDriveIndex]->curdir, "\\");
	}

	char driveLetter = g_currentDriveIndex + 'A';
	sprintf(pathName, "%c:%s", driveLetter, pDrive->curdir);
	int len = strlen(pathName);
	if (len == 3)
	{
		pathName[3] = '>';
	}
	else
	{
		pathName[len - 1] = '>';
	}

	return pathName;
}

