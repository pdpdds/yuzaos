#include <windef.h>
#include <tinythread.h>
#include <list>

using namespace tthread;
using namespace std;

int gCount = 0;
mutex gMutex;
condition_variable gCond;

// Thread function: Condition notifier
void ThreadCondition1(void* aArg)
{
	lock_guard<mutex> lock(gMutex);
	--gCount;
	gCond.notify_all();
}

// Thread function: Condition waiter
void ThreadCondition2(void* aArg)
{
	printf(" Wating...");
	lock_guard<mutex> lock(gMutex);
	while (gCount > 0)
	{
		printf(".");
		gCond.wait(gMutex);
	}
	printf("end\n");
}

int main(int argc, char* argv[])
{
	printf("Condition variable (11 + 1 threads)");

	// Set the global counter to the number of threads to run.
	gCount = 10;

	// Start the waiting thread (it will wait for gCount to reach zero).
	thread t1(ThreadCondition2, 0);

	// Start a bunch of child threads (these will decrease gCount by 1 when they
	// finish)
	list<thread*> threadList;
	for (int i = 0; i < 10; ++i)
		threadList.push_back(new thread(ThreadCondition1, 0));

	// Wait for the waiting thread to finish
	t1.join();

	// Wait for the other threads to finish
	list<thread*>::iterator it;
	for (it = threadList.begin(); it != threadList.end(); ++it)
	{
		thread* t = *it;
		t->join();
		delete t;
	}


	return 0;
}