
#include "helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "ticket_sem.h"


// pthread_barrier_t barrier;

typedef struct {
    int id;
    unsigned long acquisitions;
    int work;
    int *stop;
    sem_t *s;
    ticket_sem_t *ts;
    int use_ticket;
} arg_t;

static void small_work(int k){
    volatile unsigned x=0;
    for(int i=0;i<k;i++) x+=i;
    (void)x;
}

static void* worker(void* p){
    arg_t *a = (arg_t*)p;
    // pthread_barrier_wait(&barrier); // threads start together
    while(!*a->stop){
        if(a->use_ticket){
            // Ticket
            ticket_sem_wait(a->ts);
            small_work(a->work);
            a->acquisitions++;
            ticket_sem_post(a->ts);
        } else {
            // Posix
            sem_wait(a->s);
            small_work(a->work);
            a->acquisitions++;
            sem_post(a->s);
        }
    }
    return NULL;
}

// Compute Jain's fairness index
static double jain_index(unsigned long* x, int n){
    double sum=0, sum2=0;
    for(int i=0;i<n;i++){ sum += (double)x[i]; sum2 += (double)x[i]*(double)x[i]; }
    if(sum2==0) return 0.0;
    return (sum*sum)/(n*sum2);
}

static void usage(const char* p){
    printf("Usage: %s -t <threads> -secs <sec> -impl posix|ticket\n", p);
}

int main(int argc, char** argv){
    int threads=50, secs=5, work=5000;
    const char* impl="posix";
    for(int i=1;i<argc;i++){
        if(!strcmp(argv[i],"-t")&&i+1<argc) threads=atoi(argv[++i]);
        else if(!strcmp(argv[i],"-secs")&&i+1<argc) secs=atoi(argv[++i]);
        else if(!strcmp(argv[i],"-work")&&i+1<argc) work=atoi(argv[++i]);
        else if(!strcmp(argv[i],"-impl")&&i+1<argc) impl=argv[++i];
        else { usage(argv[0]); return 1; }
    }
    int use_ticket = !strcmp(impl,"ticket");

    pthread_t *ths = calloc(threads, sizeof(pthread_t));
    arg_t *as = calloc(threads, sizeof(arg_t));
    int stop = 0;

    pin_to_cpu0();
    // pthread_barrier_init(&barrier, NULL, threads);

    sem_t s; ticket_sem_t ts;
    if(use_ticket){ ticket_sem_init(&ts); }
    else { sem_init(&s, 0, 1); }
    
    for(int i=0;i<threads;i++){
        as[i].id=i; as[i].acquisitions=0; as[i].work=work; as[i].stop=&stop;
        as[i].s=&s; as[i].ts=&ts; as[i].use_ticket=use_ticket;
        pthread_create(&ths[i], NULL, worker, &as[i]);
    }
    
    sleep(secs); stop=1;
    for(int i=0;i<threads;i++) pthread_join(ths[i], NULL);

    unsigned long total=0; for(int i=0;i<threads;i++) total += as[i].acquisitions;
    printf("impl=%s threads=%d seconds=%d total=%lu\n", impl, threads, secs, total);
    for(int i=0;i<threads;i++) printf("th%02d=%lu\n", i, as[i].acquisitions);
    unsigned long *arr = calloc(threads, sizeof(unsigned long));
    for(int i=0;i<threads;i++) arr[i]=as[i].acquisitions;
    
    // Jain index
    printf("jain=%.4f\n", jain_index(arr, threads));

    // pthread_barrier_destroy(&barrier);

    return 0;
}
