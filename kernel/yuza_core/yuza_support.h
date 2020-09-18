#pragma once
#include <SystemAPI.h>
#include <SkyConsole.h>
#include <Debugger.h>

void PrintInfomation();
DWORD WINAPI WatchDogProc(LPVOID parameter);
DWORD WINAPI SystemIdle(LPVOID parameter);