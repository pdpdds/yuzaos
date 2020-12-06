#include <stdio.h>
#include <time.h>


int main(int argc, char** argv)
{
    time_t rawtime;
    struct tm* timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    printf("Current local time and date: %s", asctime(timeinfo));

    return 0;
}