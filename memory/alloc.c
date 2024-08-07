#include <stdio.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

#include "alloc.h"

typedef struct Map
{
    int status;
    void *start;
    int size;
} Map;


void *page;
Map page_map[PAGESIZE];

int init_alloc(){
    page = mmap(NULL,PAGESIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if(page == MAP_FAILED){
        perror(strerror(errno));
        return errno;
    }
    for(int i= 0; i < PAGESIZE; i++){
        page_map[i].status = 0;
        page_map[i].start = NULL;
        page_map[i].size = 0;
    }
    return 0;
}

int cleanup(){
    if(munmap(page,PAGESIZE) == -1){
        perror(strerror(errno));
        return errno;
    }
    return 0;
}

char *alloc(int size){
    if(size < MINALLOC || (size % MINALLOC) != 0){
        printf("block size must be multiple of %d\n",MINALLOC);
        return NULL;
    }

    // search - first fit (I am lazy)
    char *temp = page;

    int offset = -1;
    int counter = 0;
    for(int i = 0; i < PAGESIZE && counter < size; i++){
        if(page_map[i].status == 0){
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
        page_map[i+offset].status = 1;
    }

    void *start = (void *)(page + offset);
    page_map[offset].size = size;
    page_map[offset].start = start;

    return (char *)(start);
}

void dealloc(char * chunk){
    int loc;
    int found = 0;
    for(loc = 0; loc < PAGESIZE; loc++){
        if((char *)page_map[loc].start == chunk){
            found = 1;
            break;
        }
    }
    if(found){
        page_map[loc].start = NULL;
        int counter = 0;
        for(int i = loc; counter < page_map[loc].size; i++){
            page_map[i].status = 0;
            counter++;
        }
        page_map[loc].size = 0;
    }
}
