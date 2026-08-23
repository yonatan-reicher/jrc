#pragma once

#include <pthread.h>
#include <stdbool.h>

typedef struct Semaphore {
    pthread_mutex_t m;
    pthread_cond_t gt_zero_signal;
    unsigned int count;
} Semaphore;

void semaphore_init(Semaphore*, unsigned int start);
/// Must be called when no one is waiting on the semaphore :P.
void semaphore_free(Semaphore*);

/// Return the current count in the semaphore.
unsigned int semaphore_get(Semaphore*);

void semaphore_inc(Semaphore*);
void semaphore_dec(Semaphore*);
bool semaphore_try_dec(Semaphore*);
