#define MAC_SEMAPHORE_DONT_RENAME
#include "mac_semaphore.h"
#include "basic.h"

#ifdef PATCH_POSIX_SEMAPHORE

#define TODO EXPECT(false, "this is unimplemented")

int my_mac_sem_close(sem_t*) {
    TODO;
}
int my_mac_sem_destroy(sem_t* s) {
    EXPECT(s->is_unnamed, "named mac semaphores not implemented yet");
    semaphore_free(&s->data.unnamed);
    return 0;
}
int my_mac_sem_getvalue(sem_t* __restrict, int* __restrict) {
    TODO;
}
int my_mac_sem_init(sem_t* s, int shared, unsigned int start) {
    EXPECT(shared == 0, "shared not supported");
    s->is_unnamed = true;
    semaphore_init(&s->data.unnamed, start);
    return 0;
}
sem_t* my_mac_sem_open(const char*, int, ...) {
    TODO;
}
int my_mac_sem_post(sem_t* s) {
    return s->is_unnamed ? (semaphore_inc(&s->data.unnamed), 0)
                         : sem_post(&s->data.named);
}
int my_mac_sem_trywait(sem_t* s) {
    return s->is_unnamed ? semaphore_try_dec(&s->data.unnamed) ? 0 : EAGAIN
                         : sem_trywait(&s->data.named);
}
int my_mac_sem_unlink(const char*) {
    TODO;
}
int my_mac_sem_wait(sem_t* s) {
    return s->is_unnamed ? (semaphore_dec(&s->data.unnamed), 0)
                         : sem_wait(&s->data.named);
}

#endif
