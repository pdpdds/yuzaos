#include <minwindef.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <systemcall_impl.h>

void* thread_main(void*);

DWORD WINAPI Test(LPVOID parameter)
{
	printf("Thread Test!!, %s\n", parameter);
	return 0;
}

#define MAX_THREAD_COUNT 100
int main(int argc, char* argv[])
{
	//Syscall_ExitThread(0);

	int i, rc, status;

	if (argc != 2)
	{
		printf("Usage: threadtest.exe <thread count>\n");
		return 1;
	}

	int threadCount = atoi(argv[1]);

	if(threadCount > MAX_THREAD_COUNT || threadCount <= 0)
	{
		printf("thread count is abnormol. 1-100\n");
		return 1;
	}

	pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t) * threadCount);
	memset(threads, 0, sizeof(sizeof(pthread_t) * threadCount));

	for (i = 0; i < threadCount; i++)
	{
		
		pthread_create(&threads[i], NULL, &thread_main, (void*)i);
		printf("pthread_create %d, %x\n", i, threads[i]);
		
	}

	for (i = 0; i < threadCount; i++)
	{
		rc = pthread_join(threads[i], (void**)&status);
	
		if (rc == 0)
		{
			printf("Completed join with thread %d status= %d\n", i, status);
		}
	}

	//Syscall_CreateThread(Test, "Pthread", "Pthread", 16, 0);

	free(threads);

	return 0;
}

void* thread_main(void* arg)
{
	int i;
	double result = 0;

	for (i = 0; i < 1000000; i++)
	{
		result += (rand() % 100) / 5.0f;
	}
	printf("thread: %d, result = %f\n", (int)arg, result);



	return nullptr;
}