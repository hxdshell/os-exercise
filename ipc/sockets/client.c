#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <stdlib.h>

#define SOCKET_PATH "/tmp/domain_socket"

int main(int argc, char const *argv[])
{
    if(argc != 2){
        printf("[Usage] ./client <filepath>\n");
        exit(1);
    }

    // Open file
    FILE *fp;
    if((fp = fopen(*++argv,"r")) == NULL){
        printf("cannot open %s\n",*argv);
        exit(errno);
    }

    unsigned int s;
    int len,n;

    struct sockaddr_un server_addr;

    // Create Socket
    if((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        perror(strerror(errno));
        exit(errno);
    }

    // Socket init
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path,SOCKET_PATH);

    // Connect
    printf("attempting to connect...\n");
    if(connect(s,(struct sockaddr *)&server_addr,sizeof(server_addr)) == -1){
        perror(strerror(errno));
        exit(errno);
    }
    printf("connected.\n");

    // Send data to server
    char buffer[256];
    while(fgets(buffer, sizeof(buffer),fp) != NULL){
        if(send(s,buffer,256,0) ==-1){
            perror(strerror(errno));
            exit(errno);
        }
    }

    close(s);
    return 0;
}
