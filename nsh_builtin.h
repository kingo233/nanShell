/*************************************************************************
	> File Name: nsh_builtin.h
	> Author:jiangnan 
	> Mail:kingo233@outlook.com 
	> Created Time: Mon 12 Jul 2021 09:02:19 AM CST
 ************************************************************************/

#ifndef _NSH_BUILTIN_H
#define _NSH_BUILTIN_H
int nsh_exit(char* arg);
int nsh_pwd(char* arg);
int nsh_cd(char* arg);
int is_builtin(char*);
int nsh_run(int idx,char * arg);
#endif
