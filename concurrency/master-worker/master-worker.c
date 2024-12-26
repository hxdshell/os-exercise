#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

/*
 M - how many no. to produce
 N - Max size of buffer for produced no.
 C - No. of worker threads
 P - No. of master threads
*/
int M, N, C, P;
int *buffer;

int total_produced = 0;
int total_consumed = 0;
int put_place = 0;
int get_place = 0;

int buffer_count = 0;

typedef struct concurrency_control
{
    pthread_mutex_t *lock;
    pthread_cond_t *empty;
    pthread_cond_t *fill;
}concurrency_control;

void put(int value){
    buffer[put_place] = value;
    put_place = (put_place + 1) % N;
}
int get(){
    int value = buffer[get_place];
    get_place = (get_place + 1) % N;
    return value;
}

void *produce(void *args){
    concurrency_control *control = (concurrency_control *)args;
    int rc;
    while(1){
        rc = pthread_mutex_lock(control->lock);
        assert(rc == 0);

        while(buffer_count == N && total_produced < M){
            rc = pthread_cond_wait(control->empty, control->lock);
            assert(rc == 0);
        }
        if(total_produced >= M){
            pthread_cond_broadcast(control->fill);
            rc = pthread_mutex_unlock(control->lock);
            assert(rc == 0);
            break;
        }

        put(total_produced);
        printf("Produced %d\n",total_produced);
        total_produced++;
        buffer_count++;

        rc = pthread_cond_signal(control->fill);
        assert(rc == 0);
        rc = pthread_mutex_unlock(control->lock);
        assert(rc == 0);
    }

    return NULL;
}

void *consume(void *args){
    concurrency_control *control = (concurrency_control *)args;
    int value;
    int rc;

    while(1){
        rc = pthread_mutex_lock(control->lock);
        assert(rc == 0);

        while(buffer_count == 0 && total_consumed < M){
            rc = pthread_cond_wait(control->fill, control->lock);
            assert(rc == 0);
        }
        if(total_consumed >= M){
            rc = pthread_mutex_unlock(control->lock);
            assert(rc == 0);
            break;
        }

        value = get();
        printf("Consumed %d\n",value);
        total_consumed++;
        buffer_count--;

        rc = pthread_cond_signal(control->empty);
        assert(rc == 0);
        rc = pthread_mutex_unlock(control->lock);
        assert(rc == 0);
    }

    return NULL;
}

int main(int argc, char const *argv[])
{   
    // Master-Worker Thread pool where master threads (P) produces M numbers and put in the buffer of size N and worker (C) threads consume the numbers.
    concurrency_control control;
    control.empty = NULL;
    control.fill = NULL;
    control.lock = NULL;

    int rc;

    if(argc != 5){
        printf("usage: ./<exe> <M> <N> <C> <P>\n");
        return 1;
    }
    M = atoi(argv[1]);
    N = atoi(argv[2]);
    C = atoi(argv[3]);
    P = atoi(argv[4]);

    if(C == 0 || P == 0){
        printf("Number of threads cannot be 0\n");
        return 1;
    }

    pthread_t producers[P];
    pthread_t consumers[C];

    buffer = malloc(sizeof(int) * N);

    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t fill = PTHREAD_COND_INITIALIZER;
    pthread_cond_t empty = PTHREAD_COND_INITIALIZER;

    control.lock = &lock;
    control.empty = &empty;
    control.fill = &fill;

    // spawn producers
    for(int i = 0; i < P; i++){
        rc = pthread_create(&producers[i],NULL,produce,&control);
        assert(rc == 0);
    }

    //spawn consumers
    for(int i = 0; i < C; i++){
        rc = pthread_create(&consumers[i],NULL,consume,&control);
        assert(rc == 0);
    }

    //wait for producers
    for(int i = 0; i < P; i++){
        rc = pthread_join(producers[i],NULL);
        assert(rc == 0);
    }

    // wait for consumers
    for(int i = 0; i < C; i++){
        rc = pthread_join(consumers[i],NULL);
        assert(rc == 0);
    }
    
    free(buffer);
    printf("%d %d",total_produced, total_consumed);
    printf("\nDone..\n");
    return 0;
}
