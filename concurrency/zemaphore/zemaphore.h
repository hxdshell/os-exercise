#include <pthread.h>
#include <stdatomic.h>

typedef struct zemaphore {
    atomic_int z;
    pthread_cond_t cond;
    pthread_mutex_t lock;
} zem_t;

void zem_init(zem_t *, int);
void zem_up(zem_t *);
void zem_down(zem_t *);
