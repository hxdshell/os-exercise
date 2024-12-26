#include "rwlock.h"
#include <assert.h>

void InitalizeReadWriteLock(read_write_lock* rw){
    atomic_store(&rw->readers,0);
    rw->read_lock = PTHREAD_MUTEX_INITIALIZER;
    rw->write_lock = PTHREAD_MUTEX_INITIALIZER;
}
void ReaderLock(read_write_lock* rw){
    pthread_mutex_lock(&rw->read_lock);
    atomic_fetch_add(&rw->readers,1);
    if(rw->readers == 1)
        pthread_mutex_lock(&rw->write_lock);
    pthread_mutex_unlock(&rw->read_lock);
}
void ReaderUnlock(read_write_lock* rw){
    pthread_mutex_lock(&rw->read_lock);
    atomic_fetch_sub(&rw->readers,1);
    if(rw->readers == 0)
        pthread_mutex_unlock(&rw->write_lock);
    pthread_mutex_unlock(&rw->read_lock);
}
void WriterLock(read_write_lock* rw){
    pthread_mutex_lock(&rw->write_lock);
}
void WriterUnlock(read_write_lock* rw){
    pthread_mutex_unlock(&rw->write_lock);
}
