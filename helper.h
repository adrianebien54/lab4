
#ifndef HELPER_H
#define HELPER_H
#define _GNU_SOURCE
#include <time.h>
#include <stdint.h>
#include <sched.h>
#include <pthread.h>
#include <stdio.h>

static inline unsigned long long nsec_now(void){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (unsigned long long)ts.tv_sec*1000000000ull + (unsigned long long)ts.tv_nsec;
}

static inline void pin_to_cpu0(void){
#ifdef __linux__
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);
    sched_setaffinity(0, sizeof(set), &set);
#endif
#ifndef __linux__
    printf("Warning: pin_to_cpu0() not implemented for this OS (non linux device)\n");
#endif    
}

#endif
