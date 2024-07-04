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

    int n = SIZE/8;
    for(int i = 0; i < n; i++){
        sprintf(area_ptr,"%s","freeeee");
        area_ptr += 8;
    }

    return 0;
}
