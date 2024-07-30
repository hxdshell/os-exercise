#include <stdio.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

#include "alloc.h"

#define BLOCKSIZE (PAGESIZE/MINALLOC)

void *page;
int page_map[BLOCKSIZE];
int main(int argc, char const *argv[])
{
    init_alloc();
    printf("%p\n\n",page);
    char *one = alloc(8);
    char *two = alloc(16);
    char *three = alloc(8);


    cleanup();
    return 0;
}

int init_alloc(){
    page = mmap(NULL,PAGESIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if(page == MAP_FAILED){
        perror(strerror(errno));
        return errno;
    }
    memset(page_map,0,sizeof(page_map));

    printf("page created\n");
    return 0;
}

int cleanup(){
    if(munmap(page,PAGESIZE) == -1){
        perror(strerror(errno));
        return errno;
    }
    printf("\npage returned\n");
    return 0;
}

char *alloc(int size){
    if(size < MINALLOC || (size % 8) != 0){
        printf("block size must be multiple of 8\n");
        return NULL;
    }

    // search - first fit (I am lazy)
    char *temp = page;

    int offset = -1;
    int counter = 0;
    for(int i = 0; i < BLOCKSIZE && counter < size; i++){
        if(page_map[i] == 0){
            if(offset == -1)
                offset = i;

            counter++;
        }else{
            if(offset != -1){
                offset = -1;
                counter = 0;
            }
        }
    }
    
    if(counter < size)
        return NULL;

    for(int i = 0; i < size; i++){
        page_map[i+offset] = 1;
    }

    void *start = (void *)(page + offset);

    return (char *)(start);
}

void dealloc(char * chunk){
    
}
