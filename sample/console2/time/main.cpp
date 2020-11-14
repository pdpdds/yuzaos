#include <stdio.h>
#include <time.h>

int main2(int argc, char** argv)
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

int main(int argc, char** argv)
{
    time_t rawTime;
    struct tm* pTimeInfo;

    rawTime = time(NULL);                
    pTimeInfo = localtime(&rawTime);    

    printf("time_t : %lld\n", rawTime);

    int year = pTimeInfo->tm_year + 1900;    //연에는 1900을 더한다.
    int month = pTimeInfo->tm_mon + 1;    // 월에는 1을 더한다.
    int day = pTimeInfo->tm_mday;
    int hour = pTimeInfo->tm_hour;
    int min = pTimeInfo->tm_min;
    int sec = pTimeInfo->tm_sec;
    printf("timeInfo : %d-%d-%d-%d-%d-%d\n", year, month, day, hour, min, sec);

    return 0;
}
