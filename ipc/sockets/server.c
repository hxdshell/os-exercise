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
#define GREEN "\033[0;32m"
#define CYAN "\033[0;36m"
#define RESETCOLOR "\033[0m" 

int main(int argc, char const *argv[])
{
    unsigned int s,s2;
    int len,n;
    char buffer[256];

    struct sockaddr_un server_addr, client_addr;

    // Create Socket
    if((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        perror(strerror(errno));
        exit(errno);
    }

    // Socket init
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path,SOCKET_PATH);
    unlink(server_addr.sun_path);

    // Bind socket
    if(bind(s,(struct sockaddr *)&server_addr,sizeof(server_addr)) == -1){
        perror(strerror(errno));
        exit(errno);
    }

    // Listen
    if(listen(s, 2) == -1){
        perror(strerror(errno));
        exit(errno);
    }

    // connect to clients
    int done;
    while(1){
        printf("\nwaiting for a connection...\n");
        printf("%sCTRL + C to stop the server%s\n",CYAN,RESETCOLOR);

        len = sizeof(client_addr);
        s2 = accept(s,(struct sockaddr *)&client_addr, &len);
        if(s2 == -1){
            perror(strerror(errno));
            exit(errno);
        }
        printf("%sconnected.%s\n", GREEN,RESETCOLOR);

        done = 0;
        do{
            n = recv(s2,buffer,256,0);
            if(n == -1){
                perror("Receive Error");
                done = 1;
                exit(errno);
            }else if(n == 0){
                break;
            }

            printf("%s[-]%s %s\n",GREEN, RESETCOLOR,buffer);

        }while(!done);
        close(s2);
    }


    return 0;
}
