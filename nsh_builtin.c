/*************************************************************************
	> File Name: nsh_builtin.c
	> Author:jiangnan 
	> Mail:kingo233@outlook.com 
	> Created Time: Sun 11 Jul 2021 10:57:30 PM CST
 ************************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

const char* builtin_strs[] = {
    "exit",
    "cd",
    "pwd"
};

int nsh_exit(char* arg){
    printf("exit\n");
    exit(0);
}
int nsh_cd(char* path){
    if(chdir(path) < 0){
        perror("cd");
        return 1;
    }
    else return 0;
}
int nsh_pwd(char* arg){
    char buf[1024];
    getcwd(buf,1024);
    printf("%s\n",buf);
    return 0;
}
int num_builtins(){
    return sizeof(builtin_strs) / sizeof(char*);
}

int is_builtin(char* cmd){
    int num = num_builtins();
    for(int i = 0; i < num;i++){
        if(strcmp(cmd,builtin_strs[i]) == 0)return i;
    }
    return -1;
}
int (*builtin_ptrs[])(char*) = {
    &nsh_exit,
    &nsh_cd,
    &nsh_pwd
};
int nsh_run(int idx,char* arg){
    return builtin_ptrs[idx](arg);    
}
