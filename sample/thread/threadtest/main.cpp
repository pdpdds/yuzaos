#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <math.h>

void* thread_main(void*);

#define MAX_THREAD_COUNT 100
int main(int argc, char* argv[])
{
	
	int i, rc, status;

	if (argc != 2)
	{
		printf("Usage: test_posix.exe <thread count>\n");
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

	free(threads);

	return 0;
}

void* thread_main(void* arg)
{
	double result = 0;

	for (int i = 0; i < 1000000; i++)
	{
		result += (rand() % 100) / 5.0f;
	}
	printf("thread: %d, result = %f\n", (int)arg, result);

	return nullptr;
}