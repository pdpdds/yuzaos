#include <stdio.h>
#include <systemcall_impl.h>
#include <time.h>

int main(int argc, char** argv)
{
	printf("hello world!!\n");

	time_t rawtime;
	struct tm* timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	printf("Current local time and date: %s", asctime(timeinfo));

	//Syscall_Sleep(1000);

	return 0;
}
