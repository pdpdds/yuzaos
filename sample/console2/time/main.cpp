#include <stdio.h>
#include <time.h>

int main(int argc, char** argv)
{
    int ret;
    struct tm info;
    char buffer[80];

    info.tm_year = 2020 - 1900;
    info.tm_mon = 5 - 1;
    info.tm_mday = 3;
    info.tm_hour = 20;
    info.tm_min = 37;
    info.tm_sec = 20;
    info.tm_isdst = -1;

    ret = mktime(&info);
    if (ret == -1)
    {
        printf("Error: unable to make time using mktime\n");
    }
    else
    {
        strftime(buffer, sizeof(buffer), "%c", &info);
        printf(buffer);
    }

    return 0;
}
