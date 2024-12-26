#include "rwlock.h"
#include <assert.h>

void InitalizeReadWriteLock(read_write_lock* rw){
    atomic_store(&rw->writers_waiting,0);
    atomic_store(&rw->readers,0);
    atomic_store(&rw->writer_active,0);

    rw->global_lock = PTHREAD_MUTEX_INITIALIZER;
    rw->ready = PTHREAD_COND_INITIALIZER;
}
void ReaderLock(read_write_lock* rw){
    pthread_mutex_lock(&rw->global_lock);
    while(rw->writers_waiting > 0 || rw->writer_active ==1)
        pthread_cond_wait(&rw->ready,&rw->global_lock);
    atomic_fetch_add(&rw->readers,1);
    pthread_mutex_unlock(&rw->global_lock);
}
void ReaderUnlock(read_write_lock* rw){
    pthread_mutex_lock(&rw->global_lock);
    atomic_fetch_sub(&rw->readers,1);
    if(rw->readers == 0)
        pthread_cond_broadcast(&rw->ready);
    pthread_mutex_unlock(&rw->global_lock);
}
void WriterLock(read_write_lock* rw){
    pthread_mutex_lock(&rw->global_lock);
    atomic_fetch_add(&rw->writers_waiting,1);
    while (rw->readers > 0 || rw->writer_active == 1)
        pthread_cond_wait(&rw->ready,&rw->global_lock);
    atomic_fetch_sub(&rw->writers_waiting,1);
    atomic_store(&rw->writer_active,1);
    pthread_mutex_unlock(&rw->global_lock);
}
void WriterUnlock(read_write_lock* rw){
    pthread_mutex_lock(&rw->global_lock);
    atomic_store(&rw->writer_active,0);
    pthread_cond_broadcast(&rw->ready);
    pthread_mutex_unlock(&rw->global_lock);
}
