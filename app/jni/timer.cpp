

#include "timer.h"
#include <sys/time.h>

timeval start_time;

void timer_init()
{
    start_time.tv_sec    = 0;
    start_time.tv_usec   = 0;
    gettimeofday(&start_time, NULL);
}

double get_elapsed_time()
{
    timeval now;
    gettimeofday(&now, NULL);
    float seconds      = (now.tv_sec - start_time.tv_sec);
    float milliseconds = (float(now.tv_usec - start_time.tv_usec)) / 1000000.0f;
    return seconds + milliseconds;
}