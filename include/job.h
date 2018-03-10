#ifndef __JOB_H_
#define __JOB_H_
#include<sys/types.h>
enum RedirectType{RedirectRead,RedirectWrite,RedirectAppend};
typedef struct{
 enum RedirectType redirect;//重定向文件描述符
 int fd;//文件描述符
}Redirection;

typedef struct {
pid_t pid;//进程的pid
char **args;//进程执行的参数
Redirection *redirects;//重定向文件描述符
int redirect_num;//重定向文件描述符的个数
}Program;//一个进程执行

typedef struct{
char *cmd;//执行的命令
int progs_num;//命令行的参数个数
Program *progs;//命令行的参数
pid_t pgid;//整个minshell下的进程组的pid
}Job;

extern Job* create_job(char *cmd);
extern void destory_job(Job *job);
extern Program* create_program(char **arg);
extern void destory_program(Program *prog);
extern int add_program(Job *job,Program *prog);

extern Redirection* creat_redirect(int fd,enum RedirectType type);
extern void destory_redirect(Redirection *r);
extern void add_redirection(Program *prog,Redirection *r);

#endif 
