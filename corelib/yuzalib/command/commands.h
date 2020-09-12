#pragma once

///////////////////////////////////////////////////////////////////////////////
// Your title here
///////////////////////////////////////////////////////////////////////////////

//저장장치는 최대 26개
#define STORAGE_DEVICE_MAX 26

typedef struct tag_DOS_Drive
{
	char curdir[MAX_PATH];
	char info[256];

}DOS_Drive;

long CmdCls (char *szCommand);
long CmdKill(char *szCommand);
long CmdProcessList(char *szCommand);
long cmdMemState(char *szCommand);
long cmdCreateWatchdogTask(char *szCommand);
long cmdTaskCount(char *szCommand);
long cmdGlobalState(char *szCommand);
long CmdExec(char *szCommand);
long cmdGUI(char *szCommand);
long cmdCD(char *szCommand);
long cmdSwitchGUI(char *szCommand);
long cmdPCI(char *szCommand);
long cmdDir(char *szCommand);
long cmdCallStack(char *szCommand);
long cmdJpeg(char *szCommand);
long cmdDebug(char* szCommand);