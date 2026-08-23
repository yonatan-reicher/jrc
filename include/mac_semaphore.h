#pragma once

#include <semaphore.h>

// I am not sure but I think all apple products are missing semaphores
#ifdef __APPLE__
#define PATCH_POSIX_SEMAPHORE
#endif

#ifdef PATCH_POSIX_SEMAPHORE

#include "my_semaphore.h"

/// Apple does support named semaphores because they are trolling.
typedef sem_t named_sem_t;

typedef struct my_mac_sem_t {
    /// When
    bool is_unnamed;
    union {
        sem_t named;
        Semaphore unnamed;
    } data;
} my_mac_sem_t;

#define sem_t my_mac_sem_t

int my_mac_sem_close(sem_t*);
int my_mac_sem_destroy(sem_t*);
int my_mac_sem_getvalue(sem_t* __restrict, int* __restrict);
int my_mac_sem_init(sem_t*, int, unsigned int);
sem_t* my_mac_sem_open(const char*, int, ...);
int my_mac_sem_post(sem_t*);
int my_mac_sem_trywait(sem_t*);
int my_mac_sem_unlink(const char*);
int my_mac_sem_wait(sem_t*);

#ifndef MAC_SEMAPHORE_DONT_RENAME
// clang-format off
#define     sem_close       my_mac_sem_close
#define     sem_destroy     my_mac_sem_destroy
#define     sem_getvalue    my_mac_sem_getvalue
#define     sem_init        my_mac_sem_init
#define     sem_open        my_mac_sem_open
#define     sem_post        my_mac_sem_post
#define     sem_trywait     my_mac_sem_trywait
#define     sem_unlink      my_mac_sem_unlink
#define     sem_wait        my_mac_sem_wait
// clang-format on
#endif

#endif
