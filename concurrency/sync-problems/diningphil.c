#include "zemaphore.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#define NUMBER_FORKS 5
#define EAT_LIMIT 3

// ------ Synchronization -----
zem_t forks[NUMBER_FORKS];

int left(int p) { return p; }
int right(int p) { return (p + 1) % NUMBER_FORKS; }

void get_forks(int p){
    if(p == NUMBER_FORKS-1){
        zem_down(&forks[right(p)]);
        zem_down(&forks[left(p)]);
    }else{
        zem_down(&forks[left(p)]);
        zem_down(&forks[right(p)]);
    }
}

void put_forks(int p){
    zem_up(&forks[left(p)]);
    zem_up(&forks[right(p)]);
}
// -----------------------------

// ---- Philosopher Action ----
void think(int p){
    struct timespec req = {0};
    req.tv_sec = 0;
    req.tv_nsec = (rand() % 1000) * 1000000;
    
    printf("Philosopher %d is thinking...\n",p);
    nanosleep(&req,NULL);
}
void eat(int p){
    sleep(2);
}
// ----------------------------------

// ----------- TEST ------------
void* philosopher_task(void *args){
    int p = *(int *)args;
    int eat_counter = 0;
    
    while (eat_counter < EAT_LIMIT){
        think(p);

        printf("Philosopher %d is hungry!\n",p);
        get_forks(p);
        eat(p);
        put_forks(p);
        printf("Philosopher %d has finished eating.\n",p);

        eat_counter++;
    }
    printf("Philosopher %d has reached the eating limit %d\n",p,eat_counter); 
}
int main(int argc, char const *argv[]){
    srand(time(NULL));

    for(int i = 0; i < NUMBER_FORKS; i++)
        zem_init(&forks[i],1);
    
    int rc;

    const int NUMBER_PHIL = NUMBER_FORKS;
    pthread_t philosophers[NUMBER_PHIL];
    int ids[NUMBER_PHIL];

    for(int i = 0; i < NUMBER_PHIL; i++){
        ids[i] = i;
        rc = pthread_create(&philosophers[i],NULL,philosopher_task,&ids[i]);
        assert(rc == 0);
    }

    for (int i = 0; i < NUMBER_PHIL; i++)
        pthread_join(philosophers[i],NULL);    

    printf("\n---PASSED---\n");
    return 0;
}
