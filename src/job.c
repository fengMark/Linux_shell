#include"job.h"
#include<malloc.h>
#include<assert.h>
#include<stdlib.h>
#include<memory.h>
#include<string.h>
#include<stdio.h>

Job* create_job(char *cmd){
	Job *job = (Job*)malloc(sizeof(Job));
	assert(job!=NULL);
	job->cmd = (char *)malloc(sizeof(char)*strlen(cmd));
	assert(job->cmd!=NULL);
	strcpy(job->cmd,cmd);
	job->progs_num = 0;//参数个数
	job->progs = NULL;//参数列表
	return job;

}
 void destory_job(Job *job){
	assert(job!= NULL);
	free(job->progs);
	free(job->cmd);
	free(job);
}
static int arg_num(char **arg){//仅限文件內部使用，统计命令行参数个数
	int i =0;
	char *start = arg[0];
	while(start!=NULL){
		i++;
		start = arg[i];
	}
	return i;
}
Program* create_program(char **arg){
	Program *prog = (Program *)malloc(sizeof(Program));
	prog->redirect_num = 0;
	prog->redirects = NULL;
	assert(prog!=NULL);
	int counter = arg_num(arg);//统计命令行参数的个数
	prog->args = (char **)calloc(counter+1,sizeof(char *));//
//counter加一的原因：指針數組後面要加NULL
	int i;
	for(i =0;i<counter;i++){//对命令行参数初始化
	int len = strlen(arg[i]);
	prog->args[i] = (char *)malloc(len);
	assert(prog->args[i]!=NULL);
	strcpy(prog->args[i],arg[i]);//存放每一个命令行参数的元素
	}
	prog->args[i]=NULL;//指针数组的后一个元素必须加上NULL
	return prog;
}
void destory_program(Program *prog){
	assert(prog!=NULL);
	int i=0;
	while(prog->args[i]!=NULL){
		free(prog->args[i]);//释放掉命令行参数
		i++;
	}
	free(prog->redirects);
	free(prog->args);//释放命令行参数的指针数组
	free(prog);
}
 int add_program(Job *job,Program *prog){
	Program *ps =(Program *)malloc(sizeof(Program)*(job->progs_num+1));
//流出空间存放NULL
	memcpy(ps,job->progs,job->progs_num*sizeof(Program));
	ps[job->progs_num++] = *prog;
	free(job->progs);//释放掉原始的
	job->progs = ps;
}

Redirection* creat_redirect(int fd,enum RedirectType type){
	Redirection *r = (Redirection *)calloc(1,sizeof(Redirection));
	assert(r!=NULL);
	r->fd = fd;
	r->redirect = type;
	return r;
}
void destory_redirect(Redirection *r){
	assert(r!=NULL);
	free(r);
}
void add_redirection(Program *prog,Redirection *r){
	Redirection *rs = (Redirection*)calloc(prog->redirect_num+1,
					sizeof(Redirection));
	assert(rs!=NULL);
	if(prog->redirects!=NULL){
		memcpy(rs,prog->redirects,prog->redirect_num*sizeof(Redirection));
		free(prog->redirects);
	}
	prog->redirects = rs;
	memcpy(&prog->redirects[prog->redirect_num],r,sizeof(Redirection));
	prog->redirect_num += 1;
}

 

