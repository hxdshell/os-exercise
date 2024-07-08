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
    const char *producer_pipe = "/tmp/producer_pipe";
    const char *consumer_pipe = "/tmp/consumer_pipe";

    const int SIZE = 4096; 

    int shm_fd;
    void *area_ptr;

    shm_fd = shm_open(shared_mem_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if(shm_fd == -1){
        perror(strerror(errno));
        exit(errno);
    }

    area_ptr = mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED ,shm_fd, 0);
    if(area_ptr == (void *)-1){
        perror(strerror(errno));
        printf("mmap failed!\n");
        exit(errno);
    }

    // make a named pipe (consumer)
    if(mkfifo(consumer_pipe, 0777) == -1){
        if(errno != EEXIST){
            perror(strerror(errno));
            exit(errno);
        }
    }

    // producer pipe for reading
    int fdp;
    fdp = open(producer_pipe, O_RDONLY);
    if(fdp == -1){
        perror(strerror(errno));
        exit(errno);
    }

    // consumer (this) pipe for writing
    int fdc;
    fdc = open(consumer_pipe, O_WRONLY);
    if(fdc == -1){
        perror(strerror(errno));
        exit(errno);
    }

    int offset;
    void *ptr;
    for(int i = 0; i < 15; i++){
        read(fdp,&offset,sizeof(int));

        ptr = (void *)(area_ptr + offset);
        printf("%s\n",(char *)ptr);
        sleep(1);
        sprintf(ptr,"%s","freeeee");
        write(fdc,&offset, sizeof(int));
        printf("Consumed %s\n",(char *)ptr);
    }

    close(fdc);
    close(fdp);    
    return 0;
}
