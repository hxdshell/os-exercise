#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

/*
 Master-Worker Thread pool where master threads produces M numbers and worker threads consume the numbers.
*/
int M, N, C, P;
int *buffer;
int place = 0;
int count = 0;

typedef struct concurrency_control
{
    pthread_mutex_t *lock;
    pthread_cond_t *empty;
    pthread_cond_t *fill;
}concurrency_control;

void put(int value){
    buffer[place++] = value;
    count++; 
}
void *produce(void *args){
    concurrency_control *control = (concurrency_control *)args;
    
    while(count < M){
        int rc = pthread_mutex_lock(control->lock);
        assert(rc == 0);

        put(count);

        rc = pthread_mutex_unlock(control->lock);
        assert(rc == 0);
    }

    return NULL;
}



int main(int argc, char const *argv[])
{
    // M - how many no. to produce
    // N - Max size of buffer for produced no.
    // C - No. of worker threads
    // P - No. of master threads

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

    pthread_t producers[P];

    buffer = malloc(sizeof(int) * N);
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

    control.lock = &lock;

    for(int i = 0; i < P; i++){
        rc = pthread_create(&producers[i],NULL,produce,&control);
        assert(rc == 0);
    }

    for(int i = 0; i < P; i++){
        rc = pthread_join(producers[i],NULL);
        assert(rc == 0);
    }
    for(int i = 0; i < N; i++){
        printf("%d,",buffer[i]);
    }
    printf("\n");

    free(buffer);
    return 0;
}
