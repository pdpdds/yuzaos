#include <stdio.h>
#include <time.h>
#include <systemcall_impl.h>

int main(int argc, char** argv)
{
    clock_t start_t, end_t, total_t;
   
    start_t = clock();
    printf("start tick = %ld\n", start_t);
    
    Syscall_Sleep(2000);

    end_t = clock();
    printf("end tick = %ld\n", end_t);

    total_t = (end_t - start_t) / CLOCKS_PER_SEC;
    printf("CPU Time : %f\n", (float)total_t);
    
	return 0;
}