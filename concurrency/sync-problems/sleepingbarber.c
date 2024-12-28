// N seats : N-1 in waiting room and 1 in the barber room
// if there are no customers in the waiting room and the "barber chair", the barber goes to sleep
// if a customer enters and sees the barber sleeping, then the customer wakes him up
// if a customer enters and all seats are occupied then he leaves. If a chair is available in the waiting room, then he waits.
// after finishing one haircut the barber picks a customer from waiting room to get a haircut.

#include <zemaphore.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <stdatomic.h>

#define N 3     
#define NUMBER_CUSTOMERS 12

int occupied_wr_seats = 0;
int unhappy_customers = 0;

atomic_int close_shop = 0;

zem_t wake_me_up;
zem_t lock_wr_seats;
zem_t barber_ready;
zem_t lock_unhappy_customers;

void get_hair_cut(int c){
    sleep(rand() % 4);
    printf("\033[0;32mCustomer %d got a new haircut.\033[0m\n",c);
    zem_down(&lock_wr_seats);
    occupied_wr_seats--;
    printf("%d seats occupied\n",occupied_wr_seats);
    zem_up(&lock_wr_seats);
}

void *barber(void *args){
    while (!atomic_load(&close_shop)){
        zem_down(&lock_wr_seats);
        if(occupied_wr_seats == 0){
            zem_up(&lock_wr_seats);
            printf("\033[0;44mNo customers...Barber is going to sleep zzzzz\033[0m\n");
            zem_down(&wake_me_up);
        }else{
            zem_up(&lock_wr_seats);
            zem_up(&barber_ready);
        }
    }
    return NULL;
}
void *customer(void *args){
    int c = *(int *)args;

    zem_down(&lock_wr_seats);
    if(occupied_wr_seats == 0){
        occupied_wr_seats++;
        zem_up(&lock_wr_seats);
        printf("Customer %d wakes up the barber\n",c);
        zem_up(&wake_me_up);     // wake up barber
    }else if(occupied_wr_seats == N){
        printf("\033[0;31mCustomer %d left without getting a haircut\033[0m\n",c);
        zem_up(&lock_wr_seats);

        zem_down(&lock_unhappy_customers);
        unhappy_customers++;
        zem_up(&lock_unhappy_customers);
        return NULL;
    }else{
        occupied_wr_seats++;
        printf("Customer %d is waiting...\n",c);
        zem_up(&lock_wr_seats);
    }
    zem_down(&barber_ready);
    get_hair_cut(c);

    return NULL;
}

int main(int argc, char const *argv[]){
    int rc;
    srand(time(NULL));

    zem_init(&lock_wr_seats,1);
    zem_init(&lock_unhappy_customers,1);
    zem_init(&wake_me_up,0);
    zem_init(&barber_ready,0);
    
    pthread_t barber_thread;
    pthread_t customers[NUMBER_CUSTOMERS];
    
    rc = pthread_create(&barber_thread,NULL,barber,NULL);
    assert(rc == 0);

    int ids[NUMBER_CUSTOMERS];
    for(int i = 0; i < NUMBER_CUSTOMERS; i++){
        ids[i] = i;
        rc = pthread_create(&customers[i],NULL,customer,&ids[i]);
        assert(rc == 0);
        sleep(rand() % 2);
    }

    for(int i = 0; i < NUMBER_CUSTOMERS; i++)
        pthread_join(customers[i],NULL);

    atomic_store(&close_shop,1);
    zem_up(&wake_me_up);
    pthread_join(barber_thread,NULL);

    printf("\n%d customer(s) were not able to get a haircut\n",unhappy_customers);
    printf("---The shop is now closed.---\n\n");
    return 0;
}
