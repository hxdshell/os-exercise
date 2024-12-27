#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include "zemaphore.h"

void zem_init(zem_t *s, int value) {
    atomic_store(&s->z,value);
    pthread_cond_init(&s->cond, NULL);
    pthread_mutex_init(&s->lock, NULL);
}

void zem_down(zem_t *s) {
    pthread_mutex_lock(&s->lock);
    while(s->z <= 0)
        pthread_cond_wait(&s->cond,&s->lock);
    
    atomic_fetch_sub(&s->z,1);
    pthread_mutex_unlock(&s->lock);    
}

void zem_up(zem_t *s) {
    pthread_mutex_lock(&s->lock);
    atomic_fetch_add(&s->z,1);
    pthread_cond_signal(&s->cond);
    pthread_mutex_unlock(&s->lock);
}
