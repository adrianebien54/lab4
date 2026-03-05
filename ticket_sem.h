
// ticket_sem_starter.h — very small FIFO-style ticket semaphore (starter)
#ifndef TICKET_SEM_STARTER_H
#define TICKET_SEM_STARTER_H
#include <semaphore.h>
#include <pthread.h>

// Simple ticket semaphore using a ticket counter and now_serving counter.
// We protect the counters with a mutex to keep code very simple.

typedef struct {
    unsigned long next_ticket;
    unsigned long now_serving;
    pthread_mutex_t mtx;
} ticket_sem_t;

// Initialize the ticket semaphore
static void ticket_sem_init(ticket_sem_t* ts){
    ts->next_ticket = 0;
    ts->now_serving = 0;
    pthread_mutex_init(&ts->mtx, NULL);
}

// Wait: get a ticket, then spin until it's your turn
static void ticket_sem_wait(ticket_sem_t* ts){
    unsigned long my;
    pthread_mutex_lock(&ts->mtx);
    my = ts->next_ticket++;
    pthread_mutex_unlock(&ts->mtx);

    while(1){
        pthread_mutex_lock(&ts->mtx);
        if(ts->now_serving == my){
            pthread_mutex_unlock(&ts->mtx);
            break;
        }
        pthread_mutex_unlock(&ts->mtx);
        sched_yield();
    }
}

// Post: advance now_serving to let the next ticket proceed
static void ticket_sem_post(ticket_sem_t* ts){
    pthread_mutex_lock(&ts->mtx);
    ts->now_serving++;
    pthread_mutex_unlock(&ts->mtx);
}
#endif
