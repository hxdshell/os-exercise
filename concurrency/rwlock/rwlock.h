#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#ifdef __cplusplus
    #include <atomic>
    extern "C" {
        typedef std::atomic<int> atomic_int;
#else
    #include <stdatomic.h>
#endif

typedef struct read_write_lock
{
    atomic_int readers;
    pthread_mutex_t write_lock;
    pthread_mutex_t read_lock;
    
    atomic_int writers_waiting;
    pthread_cond_t ready;
    pthread_mutex_t global_lock;
    atomic_int writer_active;
}read_write_lock;

void InitalizeReadWriteLock(read_write_lock * rw);
void ReaderLock(read_write_lock * rw);
void ReaderUnlock(read_write_lock * rw);
void WriterLock(read_write_lock * rw);
void WriterUnlock(read_write_lock * rw);

#ifdef __cplusplus
    }
#endif
