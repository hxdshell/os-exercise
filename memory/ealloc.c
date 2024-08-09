#include <stdio.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

#include "ealloc.h"

#define MAXPAGES 4

typedef struct PTE
{
    int status;
    void *start;
    int size;
}PTE;


typedef struct PDE{
    void *page;
    PTE page_table[PAGESIZE];
}PDE;

PDE page_directory[MAXPAGES];
void init_alloc(void){
    for(int i = 0; i < MAXPAGES; i++)
        page_directory[i].page = NULL;

}
char *get_chunk(int size, void *page, PTE *page_table){
    char *temp = page;

    int offset = -1;
    int counter = 0;
    for(int i = 0; i < PAGESIZE && counter < size; i++){
        if(page_table[i].status == 0){
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
        page_table[i+offset].status = 1;
    }

    void *start = (void *)(page + offset);
    page_table[offset].size = size;
    page_table[offset].start = start;

    return (char *)(start);
}
char *alloc(int req_size){
    if (req_size < MINALLOC || (req_size % MINALLOC) != 0){
        printf("allocation size should at least be %d or multiple of it.\n",MINALLOC);
        exit(1);
    }

    for(int i = 0; i < MAXPAGES; i++){
        if (page_directory[i].page == NULL){
            page_directory[i].page =  mmap(NULL,PAGESIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

            for(int j= 0; j < PAGESIZE; j++){
                page_directory[i].page_table[j].status = 0;
                page_directory[i].page_table[j].start = NULL;
                page_directory[i].page_table[j].size = 0;

            }
        }

        char *chunk = get_chunk(req_size, page_directory[i].page, page_directory[i].page_table);

        if(chunk != NULL)
            return chunk;
    }
    return NULL;
}

void dealloc(char *chunk){
    for(int i = 0; i < MAXPAGES; i++){
        if(page_directory[i].page <= (void *)chunk && (void *)(page_directory[i].page + PAGESIZE) > (void *)chunk){
            int loc;
            int found = 0;
            for(loc = 0; loc < PAGESIZE; loc++){
                if((char *)page_directory[i].page_table[loc].start == chunk){
                    found = 1;
                    break;
                }
            }
            if(found){
                page_directory[i].page_table[loc].start = NULL;
                int counter = 0;
                for(int j = loc; counter < page_directory[i].page_table[loc].size; j++){
                    page_directory[i].page_table[j].status = 0;
                    counter++;
                }
                page_directory[i].page_table[loc].size = 0;
            }
        }
    }
}
void cleanup(void){
    for(int i = 0; i < MAXPAGES; i++){
        PTE *page_table;
        if(munmap(page_directory[i].page,PAGESIZE) == -1){
            perror(strerror(errno));
            exit(errno);
        }
        page_directory[i].page = NULL;
        page_table = page_directory[i].page_table;
        for(int i = 0; i < PAGESIZE; i++){
            page_table[i].status = 0;
            page_table[i].start = NULL;
            page_table[i].size = 0;
        }
    }
}