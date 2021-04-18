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
		//if (eip >= (*iter)->getMinAddress() && eip < (*iter)->getMaxAddress())
		{
			addressResolved = (*iter)->getAddressInfo(eip, objFileName, filename, lineNumber, function, resultAddress);

			if (addressResolved == true)
				break;
		}
	}

	if (iter == m_symbolList.end() || addressResolved == false)
		return false;

	bool result = (*iter)->SimpleUndecorateCPP(function, undecorateName, 256);
	
	if (result == true)
	{
		SkyConsole::Print("  %x %s!%s + 0x%x, %s", eip, (*iter)->GetModuleName(), undecorateName, eip - resultAddress, objFileName);
	}
	else
	{
		SkyConsole::Print("  %x %s!%s + 0x%x, %s", eip, (*iter)->GetModuleName(), function, eip - resultAddress, objFileName);
	}

	return true;
}

extern "C" void SetupTrapCommon(int, int);
void StackTracer::TraceStackWithSymbol(unsigned int maxFrames, unsigned int faultAddress)
{
	if (m_engineInit == false)
	{
		SkyConsole::Print("debug engine not initialized\n");
		return;
	}

	SkyConsole::Print("Stack trace:\n");

	if (faultAddress)
	{
		bool result = ResolveAddressInfo(faultAddress);

		if (result == false)
		{
			SkyConsole::Print("fault address  0x{%x}\n", faultAddress);
		}
	}

	//���� ��Ȳ
	//  ù��° �Ķ���� maxFrames
	//  TraceStackWithSymbol �Լ��� �����Ų ȣ���Լ� �����ּ�
	//  ȣ���Լ��� EBP(���� EBP �������Ͱ� �̰��� ����Ű�� �ִ�)
	unsigned int* ebp = &maxFrames - 2;
	
	bool foundSetupTrapFunc = false;
	for (unsigned int frame = 0; frame < maxFrames; ++frame)
	{
		unsigned int eip = ebp[1];

		//�Լ� �����ּҰ� 0�̸� �ݽ��� ����� ������.
		if (eip == (unsigned int)&kExitThread || eip == (unsigned int)&kmain || eip == 0 )
			break;

		// ���� ȣ���Լ��� �������������� �̵��Ѵ�.
		ebp = reinterpret_cast<unsigned int *>(ebp[0]);
		unsigned int * arguments = &ebp[2];

		if (ebp[0] == 0)
			break;

		bool found = false;
		if (faultAddress && (eip == (unsigned int)&SetupTrapCommon + 0x07) && foundSetupTrapFunc == false)
		{
			found = true;
		}

		if (found == true)
		{
			foundSetupTrapFunc = true;
			continue;
		}

		if (faultAddress && foundSetupTrapFunc == false && found == false)
			continue;
	
		// �ɹ��������� ���� �ش��ּ��� �Լ��̸� ���� ���� ���´�.
		bool result = ResolveAddressInfo(eip);

		if (result == false)
		{
			SkyConsole::Print("  0x{%x}\n", eip);			
		}

	}
}

//������� ������ ���� ���������� ȣ��� �Լ��� ǥ����� �ʴ´�.
//������ TraceStackWithSymbol �޼ҵ�� �����غ��� �� �� �ִ�.
//�ذ�å
//1. Ÿ�� ���μ����� ������ ���丮�� ��ü�� �� EIP �������͸� �����Ѵ�.
//2. ������ ���丮�� ������� �����ѵ� EIP �ּҿ� �ش��ϴ� �ɹ��� ��´�.
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
				//�Լ� �����ּҰ� 0�̸� �ݽ��� ����� ������.
				break;

			// ���� ȣ���Լ��� �������������� �̵��Ѵ�.
			ebp = reinterpret_cast<unsigned int *>(ebp[0]);
			unsigned int * arguments = &ebp[2];

			if (m_symbolInit == true && m_pMapReader != nullptr)
			{
				// �ɹ��������� ���� �ش��ּ��� �Լ��̸� ���� ���� ���´�.
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

bool StackTracer::AlreadySymbolLoaded(const char* symbolName)
{
	auto iter = m_symbolList.begin();

	for (iter; iter != m_symbolList.end(); iter++)
	{
		if (strcmp((*iter)->GetModuleName(), symbolName) == 0)
			return true;
	}

	return false;
}

//����׿��� ����� �ε��Ѵ�.
bool StackTracer::AddSymbol(const char* symbolName, unsigned int actualLoadedAddress)
{				
	I_MapFileReader* reader = GetDebugEngineDLLInterface();

	if (reader == nullptr)
	{
		kDebugPrint("Symbol Load Fail : %s\n", symbolName);
		return false;
	}

	if (AlreadySymbolLoaded(symbolName))
		return true;
	
	bool result = reader->readFile((char*)symbolName);
	if (result == false)
	{
		//printf("Add SymbolFile Fail!!, %s\n", symbolName);
		return false;
	}
		
	if (actualLoadedAddress == 0)
	{
		actualLoadedAddress = reader->getPreferredLoadAddress();
	}
	reader->setLoadAddress(actualLoadedAddress);

	m_symbolList.push_back(reader);
	
	return true;
}