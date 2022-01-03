/*************************************************************************
> File Name: main.c
> Author:jiangnan 
> Mail:kingo233@outlook.com 
> Created Time: Sun 11 Jul 2021 04:13:59 PM CST
************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "nsh_builtin.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <pwd.h>

#define PIPE_IN 2
#define PIPE_OUT 1
extern char ** environ;

const int INI_BUFSIZE = 1024;
char * nsh_getline(){
    int bufsize = INI_BUFSIZE;
    char * line = (char*)malloc(INI_BUFSIZE);
    if(!line){
        fprintf(stderr,"nsh:malloc failed.\n");
        exit(1);
    }

    char c;
    int pos = 0;
    while(1){
        c = getchar();
        if(c == EOF || c == '\n'){
            line[pos] = '\0';
            return line;
        }
        else if(c == '|' || c == '<' || c == '>'){
            line[pos++] = ' ';
            line[pos++] = c;
            line[pos++] = ' ';
        }
        else{
            line[pos++] = c;
        }
        // | < > leads to write 3 chars 
        if(pos >= bufsize + 2){
            bufsize += INI_BUFSIZE;
            void * newptr = realloc(line,bufsize);
            if(!newptr){
                free(line);
                fprintf(stderr,"nsh:realloc failed.\n");
                exit(1);
            }
            line = newptr;
        }
    }
}
const int NSH_TOKEN_BUFSIZE = 64;
const char * NSH_TOK_DELIM = "\t\r\n\a ";
char ** nsh_spilt_line(char* line){
    int bufsize = NSH_TOKEN_BUFSIZE;
    char ** tokens = malloc(bufsize * sizeof(char*));
    char * token;
    if(!tokens){
        fprintf(stderr,"nsh:malloc failed.\n");
        exit(1);
    }

    token = strtok(line,NSH_TOK_DELIM);
    int pos = 0;
    while(token != NULL){
        tokens[pos++] = token;
        if(pos >= bufsize){
            bufsize += NSH_TOKEN_BUFSIZE;
            void * newptr = realloc(tokens,bufsize * sizeof(char*));
            if(!newptr){
                free(tokens);
                fprintf(stderr,"nsh:realloc failed.\n");
                exit(1);
            }
            tokens = (char**) newptr;
        }
        token = strtok(NULL,NSH_TOK_DELIM);
    }
    tokens[pos] = NULL;
    return tokens;
}
void execute(char** argv,char** files,int piped,int now){
    char pipename[20];
    pid_t pid = fork();
    if(pid < 0){
        perror("fork");
        return ;
    }
    if(!pid){
        if(files[0] != NULL){
            close(0);
            int fd = open(files[0],O_RDONLY); 
            if(fd < 0){
                perror("open");
                exit(1);
            }
        }
        if(files[1]){
            close(1);
            int fd = open(files[1],O_WRONLY | O_CREAT | O_TRUNC,0666);
            if(fd < 0){
                perror("open");
                exit(1);
            }
        }
        if(piped & PIPE_OUT){
            sprintf(pipename,"%s%d",".nshpipe",now);
            mkfifo(pipename,0666);
            close(1);
            int fd = open(pipename,O_WRONLY);
        }
        if(piped & PIPE_IN){
            usleep(1000);
            sprintf(pipename,"%s%d",".nshpipe",now - 1);
            close(0);
            int fd = open(pipename,O_RDONLY);
            if(fd < 0){
                printf("%s\n",pipename);
                perror("open");
            }
        }
        if(execvp(argv[0],argv) < 0){
            perror("");
            exit(1);
        }
    }
    else{
        if(piped & PIPE_OUT)return;
        else {
            int status;
            wait(&status);
        }
    }
}
void parse(char ** args){
    int i = 0;
    int cmd = 1;
    char ** argv[10];
    int argc[10] = {0};
    char * files[10][2] = {0};
    int piped[10] = {0};
    int numofprocess = -1;
    const int maxargc = 32;
    for(char * temp = args[i];temp;i++,temp = args[i]){
        if(!strcmp(temp,"<")){
            i++;
            files[numofprocess][0] = args[i];
        } 
        else if(!strcmp(temp,">")){
            i++;
            files[numofprocess][1] = args[i];
        }
        else if(!strcmp(temp,"|")){
            cmd = 1; 
            piped[numofprocess] |= PIPE_OUT;
        }
        else{
            if(cmd){
                if(numofprocess >= 0){
                    argv[numofprocess][argc[numofprocess]] = NULL;
                }
                numofprocess++;
                if(numofprocess > 0 && (piped[numofprocess - 1] & PIPE_OUT))piped[numofprocess] |= PIPE_IN; 
                argv[numofprocess] = (char**) malloc(maxargc * sizeof(char*)); 
                argv[numofprocess][argc[numofprocess]++] = temp;
                cmd = 0;
            }
            else{
                argv[numofprocess][argc[numofprocess]++] = temp;
            }
        }
    }

    if(numofprocess >= 0)argv[numofprocess][argc[numofprocess]] = NULL;
    argv[numofprocess + 1] = NULL;
    int re_code = 0;
    int num = 0;
    for(i = 0;argv[i];i++,num++){
        int idx = is_builtin(argv[i][0]);
        if(~idx){
            re_code = nsh_run(idx,argv[i][1]);
        }
        else{
            execute(argv[i],files[i],piped[i],i);
        }
    }
    char pipename[30];
    for(int i = 0; i < num - 1;i++){
        sprintf(pipename,"%s%d",".nshpipe",i);
        remove(pipename);
    }
    usleep(10000);
}
void get_name(struct passwd** pwd){
    *pwd = getpwuid(getuid());
}

void get_wd(char* res){
    char buf[1024];
    getcwd(buf,1024);
    int st = 0;
    for(int i = 0;buf[i];i++){
        if(buf[i] == '/')st = i;
    }
    st++;
    if(!buf[st]){
        // only /
        res[0] = '/';
        res[1] = 0;
        return;
    }
    int i;
    for(i = 0;buf[st];i++,st++){
        res[i] = buf[st];
    }
    res[i] = 0;
}
void print_welcome(){
    printf(" _   _               ____  _          _ _  \n"); 
    printf("| \\ | | __ _ _ __   / ___|| |__   ___| | | \n");
    printf("|  \\| |/ _` | '_ \\  \\___ \\| '_ \\ / _ \\ | | \n");
    printf("| |\\  | (_| | | | |  ___) | | | |  __/ | | \n");
    printf("|_| \\_|\\_,_ |_| |_| |____/|_| |_|\\___|_|_| \n");
}
void init(){
    pid_t pid = fork();
    if(pid){
        wait(NULL);
    }
    else{
        execlp("stty","stty","erase","^H",NULL);
    }
}
int main(){
    char *str;
    char buf[1024];
    init();
    print_welcome();
    struct passwd* pwd;
    while(1){
        get_wd(buf);
        get_name(&pwd);
        printf("\033[035mnsh \33[034m%s\033[0m@\033[033m%s\033[0m $ ",pwd->pw_name,buf);
        str = nsh_getline();
        char ** args = nsh_spilt_line(str);
        parse(args);
        free(args);
        free(str);
    }
    return 0;
}
