#include "my_semaphore.h"
#include "basic.h"
#include <pthread.h>

typedef Semaphore S;

void semaphore_init(S* ret, unsigned int start) {
    EXPECT(pthread_mutex_init(&ret->m, NULL) == 0, "could not create a mutex");
    EXPECT(
        pthread_cond_init(&ret->gt_zero_signal, NULL) == 0,
        "could not create a condition variable"
    );
    ret->count = start;
}

void semaphore_free(S* s) {
    if (s == NULL) return;
    pthread_mutex_destroy(&s->m);
    pthread_cond_destroy(&s->gt_zero_signal);
}

#define LOCK pthread_mutex_lock(&s->m)
#define UNLOCK pthread_mutex_unlock(&s->m)

unsigned int semaphore_get(S* s) {
    unsigned int ret;
    LOCK;
    ret = s->count;
    UNLOCK;
    return ret;
}

void semaphore_inc(S* s) {
    LOCK;
    s->count += 1;
    EXPECT(s->count != 0, "overflow occurred");
    if (s->count == 1) pthread_cond_signal(&s->gt_zero_signal);
    UNLOCK;
}

void semaphore_dec(S* s) {
    LOCK;
    while (s->count == 0) pthread_cond_wait(&s->gt_zero_signal, &s->m);
    s->count -= 1;
    UNLOCK;
}

bool semaphore_try_dec(S* s) {
    bool ret;
    LOCK;
    ret = s->count > 0 ? (s->count -= 1, true) : false;
    UNLOCK;
    return ret;
}
