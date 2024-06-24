#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>

#define MAX_ARG_LEN 128 // accurate length can be found in MAX_ARG_STRLEN (look it up)
#define MAX_DIR_LEN 1024

#define GREEN "\033[0;32m"
#define CYAN "\033[0;36m"
#define RESETCOLOR "\033[0m" 

char **tokenize(char * input, long MAX_NOTOKENS, int *last_token);
void cleanup(char **tokens);

int main(int argc, char const *argv[])
{
    long MAX_INPUT_SIZE = sysconf(_SC_ARG_MAX); // ~2MB
    long MAX_NO_TOKENS = MAX_INPUT_SIZE / MAX_ARG_LEN; // 16348

    int last_token;

    int bg_running_processes=0;
    char is_bg = 0;
    int wait_bg;

    char command[MAX_INPUT_SIZE];
    char cwd[MAX_DIR_LEN];

    char **tokens;
    int rc;
    while(1){
        // reap background process
        if(bg_running_processes > 0){
            printf("%d\n",bg_running_processes);
            wait_bg = waitpid(-1,NULL,WNOHANG);
            if(wait_bg > 0){
                printf("\nDone\t[%d]\n",wait_bg);
                bg_running_processes--;
            }
        }
        is_bg = 0;
        
        getcwd(cwd,sizeof(cwd));
        printf("%s%s%s$ ", CYAN,cwd,RESETCOLOR);
        fgets(command, sizeof(command), stdin);
        tokens = tokenize(command,MAX_NO_TOKENS,&last_token);

        if(tokens[0] != NULL){
            
            // cd command handling
            if(strcmp(tokens[0],"cd")==0){
                if(tokens[2] != NULL || tokens[1] == NULL){
                    printf("cd : invalid argument\n");
                    printf("Usage: cd <pathname>\n\n");
                }
                else if(chdir(tokens[1]) == -1){
                    printf("%s\n\n",strerror(errno));
                }
                cleanup(tokens);
                continue;
            }

            //check for '&' for background process
            if(strcmp(tokens[last_token],"&") == 0 && last_token > 0){
                is_bg = 1;
                tokens[last_token] = NULL;
                bg_running_processes++;
            }
            if(tokens[last_token] == NULL)
                printf("yes!\n");
        }
        
        rc = fork();
        if(rc < 0){
            perror("process creation failed!");
            cleanup(tokens);
            continue;
        }else if(rc == 0){
            if(execvp(tokens[0],tokens)==-1){
                printf("%s\n\n",strerror(errno));
                exit(errno);
            }
        }else{
            if(!is_bg)
                waitpid(rc,NULL,0);
            cleanup(tokens);
        }
    }

    return 0;
}

void cleanup(char **tokens){
    int i;
    for(i = 0; tokens[i] != NULL; i++){
        free(tokens[i]);
    }
    free(tokens[i]);
    free(tokens);
}

char **tokenize(char *input, long MAX_NO_TOKENS, int *last_token){
    int token_number = 0, token_index = 0, i;
    char character;
    
    char *token = (char*)malloc(MAX_ARG_LEN * sizeof(char));
    char **tokens = (char**)malloc(MAX_NO_TOKENS * sizeof(char*));

    for(i = 0; i < strlen(input); i++){
        character = input[i];
        if(character == ' ' || character == '\t' || character == '\n'){
            token[token_index] = '\0';
            if(token_index != 0){
                tokens[token_number] = (char*)malloc(strlen(token) * sizeof(char));
                strcpy(tokens[token_number++],token);
                token_index = 0;
            }
        }else{
            token[token_index++] = character;
        }
    }
    
    free(token);
    tokens[token_number] = NULL;
    *last_token = (token_number > 0) ? token_number -1 : token_number;
    return tokens;
}