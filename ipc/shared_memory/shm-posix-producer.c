#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    const char* shared_mem_name = "/OS";
    const char* producer_pipe = "/tmp/producer_pipe";
    const char* consumer_pipe = "/tmp/consumer_pipe";
    const int SIZE = 4096; 

    int shm_fd;
    void *area_ptr;

    shm_fd = shm_open(shared_mem_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if(shm_fd == -1){
        perror(strerror(errno));
        exit(errno);
    }

    if(ftruncate(shm_fd,SIZE) == -1){
        perror(strerror(errno));
        exit(errno);
    }

    area_ptr = mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED ,shm_fd, 0);
    if(area_ptr == (void *)-1){
        perror(strerror(errno));
        printf("mmap failed!\n");
        exit(errno);
    }

    // intialize shared memroy with freeeee
    void *intial_ptr = area_ptr;
    int n = SIZE/8;

    for(int i = 0; i < n; i++){
        sprintf(area_ptr,"%s","freeeee");
        area_ptr += 8;
    }
    area_ptr = intial_ptr;

    // make a named pipe
    if(mkfifo(producer_pipe, 0777) == -1){
        if(errno != EEXIST){
            perror(strerror(errno));
            exit(errno);
        }
    }

    // producer (this) pipe for writing
    int fdp;
    fdp = open(producer_pipe, O_WRONLY);
    if(fdp == -1){
        perror(strerror(errno));
        exit(errno);
    }

    // consumer pipe for reading
    int fdc;
    fdc = open(consumer_pipe, O_RDONLY);
    if(fdc == -1){
        perror(strerror(errno));
        exit(errno);
    }
    
    int offset = 0;
    int i;
    sleep(1);
    for(i = 0; i < 512; i++){
        write(fdp,&offset,sizeof(int));

        sprintf(area_ptr,"%s","OSisFUN");
        offset += 8;
        area_ptr += 8;
    }

    area_ptr = intial_ptr;
    for(; i < 1000; i++){
        // read consumed memoery location (offset)
        read(fdc,&offset,sizeof(int));

        area_ptr = (void *)(intial_ptr + offset);
        sprintf(area_ptr,"%s","OSisFUN");

        write(fdp,&offset,sizeof(int));
    }

    printf("waiting for consumer to exit...\n");
    int x = 0;
    while(1){
        x = read(fdc,&offset,sizeof(int));
        if(x == 0){
            printf("Consumer exited, Goodbye\n");
            break;
        }
    }
    return 0;
}
