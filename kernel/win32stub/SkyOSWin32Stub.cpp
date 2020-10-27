// TestEngine.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <SDL.h>
#include <windows.h> 
#include "SkyOSWin32Stub.h"
#include <iostream>
#include "../../runtime/include/platformapi.h"
#include "SkyInputHandlerWin32.h"
#include "../include/BuildOption.h"
#include <time.h>
#include <io.h>


SKYOS_MODULE_LIST g_module_list;

WIN32_VIDEO g_win32Video;
CRITICAL_SECTION g_cs;

int screen_w;
int screen_h;
SDL_Surface *screen;
SDL_Window *pWindow;
SDL_Renderer *pRenderer;
SDL_Texture *pTexture;

#pragma comment(lib, "SDL2.LIB")
FILE _iob[] = { *stdin, *stdout, *stderr };
extern "C" FILE * __cdecl __iob_func(void) { return _iob; }

#define KERNEL_HEAP_FRAME_COUNT 12800 * 2

extern SKY_PRINT_INTERFACE g_printInterface;
extern SKY_PROCESS_INTERFACE g_processInterface;

bool IsGrahphicEnable = false;

//#define SKY_PHYSICAL_MEMORY_SIZE 100000000

WIN32_STUB* GetWin32Stub()
{
	InitializeCriticalSection(&g_cs);

	FILE* file = fopen(KERNEL32_NAME, "rb");
	fseek(file, 0, SEEK_END);
	off_t fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	char* pPhysicalMemory = new char[KERNEL_HEAP_FRAME_COUNT * 4096];
	WIN32_STUB* pStub = new WIN32_STUB;
	pStub->_printInterface = &g_printInterface;
	pStub->_processInterface = &g_processInterface;
	pStub->_virtualAddress = (unsigned int)pPhysicalMemory;
	pStub->_virtualAddressSize = KERNEL_HEAP_FRAME_COUNT * 4096;
	pStub->_kernelSize = fileSize;

	time_t tm;
	time(&tm);
	//printf("%ld\n\n", tm);
	char* str = ctime(&tm);
	return pStub;
}



WIN32_VIDEO* InitWin32System(int width, int height, int bpp)
{
	int result = SDL_Init(SDL_INIT_EVERYTHING);
	//윈도우와 렌더러를 생성
	if (SDL_CreateWindowAndRenderer(width, height, 0, &pWindow, &pRenderer) < 0)
	{
		std::cout << "SDL_CreateWindowAndRenderer Error: " << SDL_GetError() << std::endl;
		return nullptr;
	}
	SDL_GetWindowSize(pWindow, &screen_w, &screen_h);
	screen = SDL_CreateRGBSurface(0, screen_w, screen_h, bpp,
		0,
		0,
		0,
		0);

	if (screen == 0)
	{
		
		std::cout << "SDL_CreateRGBSurface Error: " << SDL_GetError() << std::endl;
		return nullptr;
	}

	IsGrahphicEnable = true;

	if (bpp == 32)
	{
		pTexture = SDL_CreateTexture(pRenderer,
			SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING,
			screen_w, screen_h);
	}
	else if (bpp == 24)
	{
		pTexture = SDL_CreateTexture(pRenderer,
			SDL_PIXELFORMAT_RGB24,
			SDL_TEXTUREACCESS_STREAMING,
			screen_w, screen_h);

		/*pTexture = SDL_CreateTexture(pRenderer,
			SDL_PIXELFORMAT_RGB888,
			SDL_TEXTUREACCESS_STREAMING,
			screen_w, screen_h);*/
	}
	else if (bpp == 16)
	{
		pTexture = SDL_CreateTexture(pRenderer,
			SDL_PIXELFORMAT_RGB565,
			SDL_TEXTUREACCESS_STREAMING,
			screen_w, screen_h);
	}
	else if(bpp == 8)
	{
		pTexture = SDL_CreateTexture(pRenderer,
			SDL_PIXELFORMAT_RGB332,
			SDL_TEXTUREACCESS_STREAMING,
			screen_w, screen_h);
	}

	if (pRenderer == 0)
	{
		std::cout << "SDL_CreateTexture Error: " << SDL_GetError() << std::endl;
		return nullptr;
	}

	if (pTexture == 0)
	{
		SDL_DestroyRenderer(pRenderer);
		SDL_DestroyWindow(pWindow);
		std::cout << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << std::endl;
		return 0;
	}


	g_win32Video._frameBuffer = (unsigned int)screen->pixels;
	g_win32Video._width = width;
	g_win32Video._height = height;
	g_win32Video._bpp = bpp;

	return &g_win32Video;
}

SkyInputHandlerWin32* pInputHandler = nullptr;

bool PrintWin32GUI(char* str)
{
	if (IsGrahphicEnable == false)
		return false;

	if(pInputHandler)
		pInputHandler->Print(str);

	return true;
}

VOID CALLBACK TimerCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired) {
	//printf("TimerCallback\n");

	if(pInputHandler)
		pInputHandler->SoftwareInterrupt();
}

HANDLE g_hTimer;
HANDLE g_hTimerQueue;
I_SkyInput* g_pVirtualIO;

ORANGEOS_WIN32_DLL bool StartWin32StubTimer(I_SkyInput* pVirtualIO, unsigned int& tickCount)
{
	g_pVirtualIO = pVirtualIO;
	pInputHandler = new SkyInputHandlerWin32();
	pInputHandler->Initialize(g_pVirtualIO);

	//타이머 생성
	g_hTimerQueue = CreateTimerQueue();
	
	// 처음 시작할때 0.5초 지연, 주기 0.001초마다 호출되게
	CreateTimerQueueTimer(&g_hTimer, g_hTimerQueue, TimerCallback, NULL, 500, 10, 0);

	return true;
}

WIN32_VIDEO* g_pVideo = 0;
WIN32_VIDEO* GetFrameBufferInfo()
{
	while (g_pVideo == 0)
	{
		Sleep(100);
	}

	return g_pVideo;
		
}


DWORD WINAPI DesktopProc(LPVOID lpParam)
{
	bool running = true;
	DWORD tickCount = (DWORD)lpParam;

	g_pVideo = InitWin32System(1024, 768, 32);

	SDL_ShowCursor(0);

	/*SDL_Surface *pHellowBMP = SDL_LoadBMP("gui-chan.bmp");
	if (pHellowBMP == 0)
	{
		SDL_DestroyRenderer(pRenderer);
		SDL_DestroyWindow(pWindow);
		std::cout << "SDL_LoadBMP Error: " << SDL_GetError() << std::endl;
		return;
	}

	SDL_Texture *pTexture = SDL_CreateTextureFromSurface(pRenderer, pHellowBMP);*/

	//루프를 돌며 화면을 그린다.
	while (running)
	{
		tickCount = GetTickCount();
		//이벤트를 가져온다.
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_MOUSEMOTION)
			{
				MOUSEDATA data;

				if (event.button.button == SDL_BUTTON_LEFT)
				{
					data.bButtonStatusAndFlag = MOUSE_LBUTTONDOWN;
				}
				else if (event.button.button == SDL_BUTTON_RIGHT)
				{
					data.bButtonStatusAndFlag = MOUSE_RBUTTONDOWN;
				}
				else if (event.button.button == SDL_BUTTON_RIGHT)
				{
					data.bButtonStatusAndFlag = MOUSE_MBUTTONDOWN;
				}
				else
				{
					data.bButtonStatusAndFlag = 0;
				}

				data.bXMovement = event.motion.x;
				data.bYMovement = event.motion.y;
				data.bAbsoluteCoordinate = 1;
				g_pVirtualIO->PutMouseQueue(&data);
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				MOUSEDATA data;

				if (event.button.button == SDL_BUTTON_LEFT)
				{
					data.bButtonStatusAndFlag = MOUSE_LBUTTONDOWN;
				}
				else if (event.button.button == SDL_BUTTON_RIGHT)
				{
					data.bButtonStatusAndFlag = MOUSE_RBUTTONDOWN;
				}
				else if (event.button.button == SDL_BUTTON_RIGHT)
				{
					data.bButtonStatusAndFlag = MOUSE_MBUTTONDOWN;
				}
				else
				{
					data.bButtonStatusAndFlag = 0;
				}

				data.bXMovement = event.motion.x;
				data.bYMovement = event.motion.y;
				data.bAbsoluteCoordinate = 1;
				g_pVirtualIO->PutMouseQueue(&data);
			}
			else if (event.type == SDL_MOUSEBUTTONUP)
			{
				MOUSEDATA data;
				data.bButtonStatusAndFlag = 0;
				data.bXMovement = event.motion.x;
				data.bYMovement = event.motion.y;
				data.bAbsoluteCoordinate = 1;
				g_pVirtualIO->PutMouseQueue(&data);
			}
			//키보드 이벤트가 발생했다면
			else if (event.type == SDL_KEYDOWN)
			{

				unsigned int keycode = SDL_GetKeyFromScancode(event.key.keysym.scancode);

				if (event.key.keysym.sym == SDLK_COLON || event.key.keysym.sym == SDLK_SEMICOLON)
				{
					int j = 1;
				}

				BYTE bScancode = pInputHandler->ConvertKeycodeToScancode(keycode);

				if (bScancode != 0)
					pInputHandler->ConvertScanCodeAndPutQueue(bScancode);

				if (event.key.keysym.sym == SDLK_F12)
				{

					while (0 == ::DeleteTimerQueueTimer(g_hTimerQueue, g_hTimer, nullptr))
					{
						if (ERROR_IO_PENDING == ::GetLastError())
						{
							break;
						}

						_tprintf(_T("DeleteTimerQueueTimer Error : %lu\r\n"), ::GetLastError());
					}

					running = false;

				}
			}
			else if (event.type == SDL_KEYUP)
			{

				unsigned int keycode = SDL_GetKeyFromScancode(event.key.keysym.scancode);

				BYTE bScancode = pInputHandler->ConvertKeycodeToScancode(keycode);

				if (bScancode != 0)
				{
					bScancode = bScancode | 0x80;
					pInputHandler->ConvertScanCodeAndPutQueue(bScancode);
				}



				/*unsigned char bASCIICode;
				unsigned char bFlags;
				if (bScancode != 0)
					pInputHandler->ConvertScanCodeToASCIICode(bScancode, &bASCIICode, (bool*)&bFlags);*/


			}

			else if (event.type == SDL_QUIT)
			{
				running = false;
			}
		}


		if (screen)
		{
			SDL_RenderClear(pRenderer);

			SDL_UpdateTexture(pTexture, NULL, screen->pixels, screen->pitch);
			SDL_RenderCopy(pRenderer, pTexture, NULL, NULL);
			//렌더러의 내용을 화면에 뿌린다.
			SDL_RenderPresent(pRenderer);
		}

	}

	if (screen)
	{
		//텍스처, 렌더러, 윈도우 객체를 제거하고 SDL을 종료한다.
		SDL_DestroyTexture(pTexture);
		SDL_DestroyRenderer(pRenderer);
		SDL_DestroyWindow(pWindow);
	}

	SDL_Quit();

	delete pInputHandler;

	exit(0);

}



extern "C" void LoopWin32(I_SkyInput* pVirtualIO, unsigned int& tickCount)
{
	

	pInputHandler = new SkyInputHandlerWin32();
	pInputHandler->Initialize(pVirtualIO);

//타이머 생성
	g_hTimerQueue = CreateTimerQueue();
	
	// 처음 시작할때 0.5초 지연, 주기 0.5초마다 호출되게
	CreateTimerQueueTimer(&g_hTimer, g_hTimerQueue, TimerCallback, NULL, 500, 10, 0);

	DWORD dwThreadId = 0;
	CreateThread(NULL, 0, DesktopProc, (LPVOID)tickCount, 0, &dwThreadId);
}

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

static char * ReadAllBytes(const char * filename, int * read)
{
	ifstream ifs(filename, ios::binary | ios::ate);
	ifstream::pos_type pos = ifs.tellg();

	// What happens if the OS supports really big files.
	// It may be larger than 32 bits?
	// This will silently truncate the value/
	int length = pos;

	// Manuall memory management.
	// Not a good idea use a container/.
	char *pChars = new char[length];
	ifs.seekg(0, ios::beg);
	ifs.read(pChars, length);

	// No need to manually close.
	// When the stream goes out of scope it will close the file
	// automatically. Unless you are checking the close for errors
	// let the destructor do it.
	ifs.close();
	*read = length;
	return pChars;
}

extern "C" SKYOS_MODULE_LIST* InitSkyOSModule()
{
	memset(&g_module_list, 0, sizeof(SKYOS_MODULE_LIST));

	_finddata_t fd;
	long handle;
	int result = 1;
	handle = _findfirst(".\\*.*", &fd);  //현재 폴더 내 모든 파일을 찾는다.

	if (handle == -1)
	{		
		return &g_module_list;
	}

	while (result != -1)
	{
		//printf("File: %s\n", fd.name);

		if (fd.size > 0)
		{			
			g_module_list._moduleCount++;
		}

		result = _findnext(handle, &fd);
	}

	_findclose(handle);

	if(g_module_list._moduleCount == 0)
		return &g_module_list;

	g_module_list._module = new SKYOS_MODULE[g_module_list._moduleCount];

	handle = _findfirst(".\\*.*", &fd);  //현재 폴더 내 모든 파일을 찾는다.
	result = 1;

	int index = 0;
	while (result != -1)
	{				
		if(fd.size > 0)
		{
			int readCount = 0;
			string fileName;
			fileName += fd.name;

			if (strstr(fileName.c_str(), ".dll") || strstr(fileName.c_str(), ".cfg"))
			{

				char* pBuffer = ReadAllBytes(fileName.c_str(), &readCount);

				strcpy(g_module_list._module[index]._name, fd.name);
				g_module_list._module[index]._startAddress = (unsigned int)pBuffer;
				g_module_list._module[index]._endAddress = (unsigned int)pBuffer + readCount;
				index++;

			}
			
		}
		
		result = _findnext(handle, &fd);
	}

	_findclose(handle);

	return &g_module_list;
}

typedef BOOL(WINAPI* LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
LPFN_ISWOW64PROCESS fnIsWow64Process;

bool isWow64() {
#ifdef _WIN64
	return true;
#endif
	BOOL bIsWow64 = FALSE;

	fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
		GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

	if (NULL != fnIsWow64Process) {
		if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64)) {
			// 오류 발생시 x86으로 임의로 간주
			return false;
		}
	}
	return (bIsWow64 != FALSE);
}