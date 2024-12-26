#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include "zemaphore.h"
#include <semaphore.h>

void zem_init(zem_t *s, int value) {
    s->z = value;
    s->cond = PTHREAD_COND_INITIALIZER;
    s->lock = PTHREAD_MUTEX_INITIALIZER;
}

void zem_down(zem_t *s) {
    pthread_mutex_lock(&s->lock);
    while(s->z <= 0)
        pthread_cond_wait(&s->cond,&s->lock);
    s->z -= 1;
    pthread_mutex_unlock(&s->lock);    
}

void zem_up(zem_t *s) {
    pthread_mutex_lock(&s->lock);
    s->z += 1;
    pthread_cond_signal(&s->cond);
    pthread_mutex_unlock(&s->lock);
}
