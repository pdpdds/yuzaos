#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>
#include "GUIConsoleFramework.h"

#define DEFAULT_RESOLUTION  1

static int ticks = 0;
static Uint32 SDLCALL ticktock(Uint32 interval, void* param)
{
	++ticks;
	return (interval);
}

static Uint32 SDLCALL callback(Uint32 interval, void* param)
{
	SDL_Log("Timer %d : param = %d\n", interval, (int)(uintptr_t)param);
	return interval;
}

void TestSingleTimer()
{
	SDL_Log("Test 1 : Single Timer\n");

	int desired = DEFAULT_RESOLUTION;
	SDL_TimerID timer = SDL_AddTimer(desired, ticktock, NULL);

	//10초 대기
	SDL_Log("Waiting 10 seconds\n");
	SDL_Delay(10 * 1000);

	SDL_RemoveTimer(timer);

	//타이머 해상도와 실제 타이머 해상도 확인
	if (ticks) 
	{
		SDL_Log("Timer resolution: desired = %d ms, actual = %f ms\n", desired, (double)(10 * 1000) / ticks);
	}
}

void TestMultipleTimer()
{
	SDL_Log("Test 2 : Multiple Timer\n");
	SDL_Delay(2000);

	SDL_TimerID t1, t2, t3;

	SDL_Log("Testing multiple timers...\n");
	t1 = SDL_AddTimer(100, callback, (void*)1);
	if (!t1)
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not create timer 1: %s\n", SDL_GetError());
	t2 = SDL_AddTimer(1000, callback, (void*)2);
	if (!t2)
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not create timer 2: %s\n", SDL_GetError());
	t3 = SDL_AddTimer(2500, callback, (void*)3);
	if (!t3)
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not create timer 3: %s\n", SDL_GetError());

	/* Wait 10 seconds */
	SDL_Log("Waiting 10 seconds\n");
	SDL_Delay(10 * 1000);

	SDL_Log("Removing timer 1 and waiting 5 more seconds\n");
	SDL_RemoveTimer(t1);

	SDL_Delay(5 * 1000);

	SDL_RemoveTimer(t2);
	SDL_RemoveTimer(t3);
}

void TestPerformanceCounter()
{
	SDL_Log("Test 3 : Performance Counter\n");
	SDL_Delay(2000);
	
	Uint32 start32, now32;
	Uint64 start, now;

	start = SDL_GetPerformanceCounter();
	for (int i = 0; i < 10000000; ++i) {
		ticktock(0, NULL);
	}
	now = SDL_GetPerformanceCounter();
	SDL_Log("10 million iterations of ticktock took %f ms\n", (double)((now - start) * 1000) / SDL_GetPerformanceFrequency());
	SDL_Log("Performance counter frequency: %lld\n", (unsigned long long) SDL_GetPerformanceFrequency());
	start32 = SDL_GetTicks();
	start = SDL_GetPerformanceCounter();
	SDL_Delay(1000);
	now = SDL_GetPerformanceCounter();
	now32 = SDL_GetTicks();
	SDL_Log("Delay 1 second = %d ms in ticks, %f ms according to performance counter\n", (now32 - start32), (double)((now - start) * 1000) / SDL_GetPerformanceFrequency());
}

int main_impl(int argc, char* argv[])
{
	
	//로깅 활성화
	SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

	if (SDL_Init(SDL_INIT_TIMER) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s\n", SDL_GetError());
		return (1);
	}

	TestSingleTimer();
	TestMultipleTimer();
	TestPerformanceCounter();

	SDL_Quit();
	return (0);
}

int main(int argc, char** argv)
{
	GUIConsoleFramework framework;
	return framework.Run(argc, argv, main_impl);
}