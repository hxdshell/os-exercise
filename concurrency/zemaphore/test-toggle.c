#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>
#include "zemaphore.h"

#define NUM_THREADS 3
#define NUM_ITER 10

typedef struct data
{
  int thread_id;
  zem_t *turns;
}thread_arg;

void *justprint(void *data)
{
  thread_arg *thread_data = (thread_arg *)data;

  for(int i=0; i < NUM_ITER; i++){

    int id = (thread_data->thread_id);

    zem_down(&thread_data->turns[id]);    
    printf("This is thread %d\n", id);
    zem_up(&thread_data->turns[(id + 1) % NUM_THREADS]);

  }
  return 0;
}

int main(int argc, char *argv[])
{

  pthread_t mythreads[NUM_THREADS];

  thread_arg data[NUM_THREADS];

  zem_t turns[NUM_THREADS];

  zem_init(&turns[0],1);
  for(int i=1; i < NUM_THREADS; i++)
    zem_init(&turns[i],0);

  for(int i=0; i < NUM_THREADS; i++){
      data[i].thread_id = i;
      data[i].turns = turns;
      pthread_create(&mythreads[i], NULL, justprint, (void *)&data[i]);
  }
  
  for(int i =0; i < NUM_THREADS; i++)
    pthread_join(mythreads[i], NULL);
  
  return 0;
}
