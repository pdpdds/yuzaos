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

static inline void timersub(const struct timeval* a, const struct timeval* b, struct timeval* out)
{
    out->tv_sec = a->tv_sec - b->tv_sec;
    out->tv_usec = a->tv_usec - b->tv_usec;
    if (out->tv_usec < 0) {
        out->tv_sec--;
        out->tv_usec += 1000 * 1000;
    }
}


#ifdef __cplusplus
}
#endif
