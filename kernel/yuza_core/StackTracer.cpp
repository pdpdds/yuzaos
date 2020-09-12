#include "StackTracer.h"
#include "SkyConsole.h"
#include "ModuleManager.h"
#include "PlatformAPI.h"
#include <stringdef.h>
#include <kmalloc.h>
#include "I_MapFileReader.h"
#include "intrinsic.h"
#include "SystemAPI.h"

extern "C" BOOL kExitThread(int errorCode);
extern "C" void kmain(unsigned long magic, unsigned long addr, uint32_t imageBase);

using namespace std;

StackTracer* StackTracer::m_pStackTracer = nullptr;

typedef I_MapFileReader*(*PGetDebugEngineDLL)();
PGetDebugEngineDLL GetDebugEngineDLLInterface;


StackTracer::StackTracer()
{	
	m_engineInit = false;
}


StackTracer::~StackTracer()
{
}

bool StackTracer::ResolveAddressInfo(unsigned int eip)
{
	int lineNumber = 0;
	DWORD resultAddress = 0;
	char function[256];
	char objFileName[256];
	char filename[256];
	char undecorateName[256];

	memset(function, 0, 256);
	memset(objFileName, 0, 256);
	memset(filename, 0, 256);
	memset(undecorateName, 0, 256);

	list<I_MapFileReader*>::iterator iter = m_symbolList.begin();
	bool addressResolved = false;

	for (iter; iter != m_symbolList.end(); iter++)
	{
		addressResolved = (*iter)->getAddressInfo(eip, objFileName, filename, lineNumber, function, resultAddress);

		if (addressResolved == true)
			break;
	}

	if (iter == m_symbolList.end() || addressResolved == false)
		return false;

	bool result = (*iter)->SimpleUndecorateCPP(function, undecorateName, 256);
	
	if (result == true)
	{
		SkyConsole::Print("  %s!%s + 0x%x, %s", (*iter)->GetModuleName(), undecorateName, eip - resultAddress, objFileName);
	}
	else
	{
		SkyConsole::Print("  %s!%s + 0x%x, %s", (*iter)->GetModuleName(), function, eip - resultAddress, objFileName);
	}

	return true;
}

void StackTracer::TraceStackWithSymbol(unsigned int maxFrames)
{
	if (m_engineInit == false)
	{
		SkyConsole::Print("debug engine not initialized\n");
		return;
	}

	//스택 상황
	//  첫번째 파라메터 maxFrames
	//  TraceStackWithSymbol 함수를 실행시킨 호출함수 복귀주소
	//  호출함수의 EBP(현재 EBP 레지스터가 이값을 가리키고 있다)
	unsigned int* ebp = &maxFrames - 2;
	SkyConsole::Print("Stack trace:\n");

		
	for (unsigned int frame = 0; frame < maxFrames; ++frame)
	{
		unsigned int eip = ebp[1];

		//함수 복귀주소가 0이면 콜스택 출력을 끝낸다.
		if (eip == (unsigned int)&kExitThread || eip == (unsigned int)&kmain || eip == 0)
			break;

		// 직전 호출함수의 스택프레임으로 이동한다.
		ebp = reinterpret_cast<unsigned int *>(ebp[0]);
		unsigned int * arguments = &ebp[2];
	
		// 심벌엔진으로 부터 해당주소의 함수이름 정보 등을 얻어온다.
		bool result = ResolveAddressInfo(eip);

		if (result == false)
		{
			SkyConsole::Print("  0x{%x}\n", eip);			
		}
	}
}

//방법상의 문제로 가장 마지막으로 호출된 함수는 표기되지 않는다.
//이유는 TraceStackWithSymbol 메소드와 대조해보면 알 수 있다.
//해결책
//1. 타켓 프로세스의 페이지 디렉토리로 교체한 후 EIP 레지스터를 덤프한다.
//2. 페이지 디렉토리를 원래대로 복원한뒤 EIP 주소에 해당하는 심벌을 얻는다.
void StackTracer::TraceStackWithProcessId(int processId)
{
	int flag = DisableInterrupts();
	/*
#if SKY_EMULATOR
	Process* pProcess = ProcessManager::GetInstance()->GetCurrentTask()->m_pParent;
#else
	Process* pProcess = ProcessManager::GetInstance()->FindProcess(processId);	
#endif
	Thread* pTask = ProcessManager::GetInstance()->GetCurrentTask();

	if (pProcess == nullptr)
	{
		SkyConsole::Print("process not found!!\n");
	}
	else
	{
		if (pTask->m_pParent == pProcess)
		{
			kLeaveCriticalSection();
			TraceStackWithSymbol();
			return;
		}

		SkyConsole::Print("Stack trace:\n");
		Thread* pThread = pProcess->GetMainThread();
		unsigned int* ebp = (unsigned int*)pThread->m_contextSnapshot.ebp;		

		int lineNumber = 0;
		DWORD resultAddress = 0;
		char function[256];
		char objFileName[256];
		char filename[256];
		char undecorateName[256];

		//SkyConsole::Print("  0x%x 0x%x 0x%x\n", ebp, ebp[0], ebp[1]);

		for (unsigned int frame = 0; frame < 20; ++frame)
		{
			unsigned int eip = ebp[1];
			if (eip == 0)
				//함수 복귀주소가 0이면 콜스택 출력을 끝낸다.
				break;

			// 직전 호출함수의 스택프레임으로 이동한다.
			ebp = reinterpret_cast<unsigned int *>(ebp[0]);
			unsigned int * arguments = &ebp[2];

			if (m_symbolInit == true && m_pMapReader != nullptr)
			{
				// 심벌엔진으로 부터 해당주소의 함수이름 정보 등을 얻어온다.
				memset(function, 0, 256);
				memset(objFileName, 0, 256);
				memset(filename, 0, 256);
				memset(undecorateName, 0, 256);
				
				bool result = m_pMapReader->getAddressInfo(eip, objFileName, filename, lineNumber, function, resultAddress);
								

				if (result == true)
				{
					result = m_pMapReader->SimpleUndecorateCPP(function, undecorateName, 256);
					if (result == true)
					{
						SkyConsole::Print("  %s + 0x%x, %s", undecorateName, eip - resultAddress, objFileName);
					}
					else
					{
						SkyConsole::Print("  %s + 0x%x, %s", function, eip - resultAddress, objFileName);
					}

				}
			}
			else
			{
				SkyConsole::Print("  0x{%x}", eip);
			}
		}
	}*/

	RestoreInterrupts(flag);
}

bool StackTracer::Init(const char* moduleName)
{
	m_moduleHwnd = ModuleManager::GetInstance()->LoadPE(moduleName, true);
	GetDebugEngineDLLInterface = (PGetDebugEngineDLL)ModuleManager::GetInstance()->GetModuleFunction(m_moduleHwnd, "GetDebugEngineDLL");

	if (!GetDebugEngineDLLInterface)
	{
		kPanic("Memory Module Load Fail!!");
	}

	m_engineInit = true;

	return true;
}

//디버그엔진 모듈을 로드한다.
bool StackTracer::AddSymbol(const char* symbolFile, unsigned int loadedAddress)
{				
	I_MapFileReader* reader = GetDebugEngineDLLInterface();

	if (reader == nullptr)
	{
		kPanic("Symbol Object Create Fail!!");
	}
	
	bool result = reader->readFile((char*)symbolFile);	
	if (result == false)
	{
		kPanic("Add SymbolFile Fail!!");
	}
		
	if (loadedAddress == 0)
	{
		loadedAddress = reader->getPreferredLoadAddress();
	}
	reader->setLoadAddress(loadedAddress);		

	m_symbolList.push_back(reader);
	
	return true;
}