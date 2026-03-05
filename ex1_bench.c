// Build: gcc -O2 -pthread ex1_bench_starter.c -o ex1-bench-starter
#include "helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>

volatile int spin_flag = 0;
static void spin_lock(void){
    while(__sync_lock_test_and_set(&spin_flag, 1)) { /* busy wait */ }
}
static void spin_unlock(void){
    __sync_lock_release(&spin_flag);
}

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static sem_t g_sem;
static volatile unsigned long g_counter = 0;

typedef enum { USE_SEM, USE_MUTEX, USE_SPIN } lock_kind_t;

typedef struct {
    int work_inside;     // small dummy work inside critical section
    unsigned long iters; // total increments per thread
    lock_kind_t kind;
} task_t;

// Simulate small amount of busy work
static void small_work(int k){
    volatile unsigned x=0;
    for(int i=0;i<k;i++) x += (unsigned)i;
    (void)x;
}

static void* worker(void* arg){
    task_t* t = (task_t*)arg;
    while(g_counter < t->iters){
        // lock
        if (t->kind == USE_SEM)
            sem_wait(&g_sem);
        else if (t->kind == USE_MUTEX)
            pthread_mutex_lock(&g_mutex);
        else if (t->kind == USE_SPIN)
            spin_lock();

        // work
        if (g_counter < t->iters) {
            g_counter++;
            small_work(t->work_inside);
        }

        // unlock
        if (t->kind == USE_SEM)
            sem_post(&g_sem);
        else if (t->kind == USE_MUTEX)
            pthread_mutex_unlock(&g_mutex);
        else if( t->kind == USE_SPIN)
            spin_unlock();
    }
    return NULL;
}

static void usage(const char* p){
    printf("Usage: %s -t <threads> -iters <N> -lock sem|mutex|spin\n", p);
}

int main(int argc, char** argv){
    //default values
    int threads = 1000;
    unsigned long iters = 1000000;
    int work = 0;
    const char* lockname = "sem";

    for(int i=1;i<argc;i++){
        if(!strcmp(argv[i], "-t") && i+1<argc){ threads = atoi(argv[++i]); }
        else if(!strcmp(argv[i], "-iters") && i+1<argc){ iters = strtoul(argv[++i], NULL, 10); }
        else if(!strcmp(argv[i], "-w") && i+1<argc){ work = atoi(argv[++i]); }
        else if(!strcmp(argv[i], "-lock") && i+1<argc){ lockname = argv[++i]; }
        else { usage(argv[0]); return 1; }
    }

    lock_kind_t kind = USE_SEM;
    if (!strcmp(lockname, "mutex")) kind = USE_MUTEX;
    else if (!strcmp(lockname, "spin")) kind = USE_SPIN;

    sem_init(&g_sem, 0, 1);
    g_counter = 0;

    pthread_t *ths = (pthread_t*)calloc(threads, sizeof(pthread_t));
    task_t t = { .work_inside = work, .iters = iters, .kind = kind };

    unsigned long long t0 = nsec_now();
    for(int i=0;i<threads;i++) pthread_create(&ths[i], NULL, worker, &t);
    for(int i=0;i<threads;i++) pthread_join(ths[i], NULL);
    unsigned long long dt = nsec_now() - t0;

    double secs = (double)dt / 1e9;
    double ops_sec = (double)iters / secs;
    printf("lock=%s threads=%d iters=%lu time=%.3f s ops/sec=%.0f\n",
        lockname, threads, iters, secs, ops_sec);

    return 0;
}
