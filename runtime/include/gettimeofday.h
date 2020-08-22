#pragma once

struct timeval {
    long tv_sec;
    long tv_usec;
};

#ifdef __cplusplus
extern "C"
{
#endif
void gettimeofday (struct timeval * tp, void * dummy);


#ifdef __cplusplus
}
#endif
