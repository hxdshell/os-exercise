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
#define TOTAL_BG 64
#define TOTAL_PARALLEL_FG 64

#define GREEN "\033[0;32m"
#define CYAN "\033[0;36m"
#define RESETCOLOR "\033[0m" 

char **tokenize(char * input, long MAX_NOTOKENS, int *last_token);
void cleanup(char ***queue);
int store_new_bg_process(int *record, int n);
char ***command_separation(char *input, long MAX_NO_TOKENS, int* last_token);

void sigint_handler(int num){
    if(getpgid(0) != getpid()){
        kill(SIGINT,0);
    }
    printf("\n");
}
int main(int argc, char const *argv[])
{
    long MAX_INPUT_SIZE = sysconf(_SC_ARG_MAX); // ~2MB
    long MAX_NO_TOKENS = MAX_INPUT_SIZE / MAX_ARG_LEN; // 16348

    int last_tokens[TOTAL_PARALLEL_FG];
    int last_token;

    int bg_running_processes=0;
    char is_bg = 0;
    int wait_bg;
    int bg_process_ids[TOTAL_BG];
    memset(bg_process_ids,-1,TOTAL_BG);
    int bg_index;

    char command[MAX_INPUT_SIZE];
    char cwd[MAX_DIR_LEN];

    char **tokens;
    char ***queue;
    int rc;
    int shell_id = getpid();

    //error variables
    int dir_err,too_many_bg;


    signal(SIGINT,sigint_handler);
    while(1){
        // error var init
        dir_err = too_many_bg = 0;

        // reap background process
        if(bg_running_processes > 0){
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
        queue = command_separation(command,MAX_NO_TOKENS,last_tokens);
        
        for(int i = 0; queue[i] != NULL; i++){
            tokens = queue[i];
            last_token = last_tokens[i];

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
                    dir_err = 1;
                    break;
                }
                //check for '&' for background process
                if(strcmp(tokens[last_token],"&") == 0 && last_token > 0){
                    is_bg = 1;
                    tokens[last_token] = NULL;
                    bg_running_processes++;
                }
                //exit
                int id;
                if(strcmp(tokens[0],"exit") == 0 && last_token == 0){
                    for(int i = 0; i < TOTAL_BG && bg_running_processes > 0; i++){
                        id = bg_process_ids[i];
                        if(id != -1){
                            kill(id,SIGTERM);
                            bg_process_ids[i] = -1;
                            bg_running_processes--;
                        }
                    }
                    cleanup(queue);
                    exit(0);
                }
                
            }
            // check if we can careate a bg process or not. max TOTAL_BG
            if(is_bg){
                bg_index = store_new_bg_process(bg_process_ids,TOTAL_BG);
                if(bg_index == -1){
                    printf("\n too many background processes\n");
                    too_many_bg = 1;
                    break;
                }
            }
            rc = fork();
            if(rc < 0){
                perror("process creation failed!");
                continue;
            }else if(rc == 0){
                if(is_bg){
                    setpgid(0,0);
                }
                if(execvp(tokens[0],tokens)==-1){
                    printf("%s\n\n",strerror(errno));
                    cleanup(queue);
                    exit(errno);
                }
            }else{
                if(!is_bg){
                    int status;
                    waitpid(rc,&status,0);
                }else{
                    bg_process_ids[bg_index] = rc;
                }
            }

            // error handling
            if(dir_err || too_many_bg){
                break;
            }
        }
        cleanup(queue);
    }

    return 0;
}

void cleanup(char ***queue){
    int i,j;
    for(i = 0; queue[i] != NULL;i++){
        for(j = 0; queue[i][j] != NULL; j++){
            free(queue[i][j]);
        }
        free(queue[i]);
    }
    free(queue);
}


int store_new_bg_process(int *record, int n){
    for(int i = 0; i < n; i++){
        if(record[i] == -1)
            return i;
    }
    return -1;
}

char ***command_separation(char *input, long MAX_NO_TOKENS, int* last_tokens){
    int n = strlen(input);
    
    char ***queue = (char***)(malloc(TOTAL_PARALLEL_FG * sizeof(char**)));
    int command_no = 0;

    char buffer[n];
    int index = 0, i;
    for(i = 0; i < n; i++){
        if(input[i] == '&' && i+1 < n){
            if(input[i+1] == '&'){
                buffer[index] = '\0';
                queue[command_no] = tokenize(buffer,MAX_NO_TOKENS,&last_tokens[command_no]);
                command_no++;
                
                // reset buffer
                index = 0;
                i++;
                continue;
            }
        }
        buffer[index++] = input[i];
    }
    buffer[index++] = '\0';
    queue[command_no++] = tokenize(buffer,MAX_NO_TOKENS,&last_tokens[command_no]);
    queue[command_no] = NULL;

    return queue;
}

char **tokenize(char *input, long MAX_NO_TOKENS, int *last_token){
    int token_number = 0, token_index = 0, i, command_number = 0;
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
    if(last_token != NULL)
        *last_token = (token_number > 0) ? token_number -1 : token_number;
    return tokens;
}