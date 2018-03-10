#include<unistd.h>
#include<string.h>
#include<fcntl.h>
#include<stdlib.h>
#include<memory.h>
#include<assert.h>
#include"job.h"
#include<malloc.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/wait.h>
char *prompt = "mshell>";
#define MAX_COMMAND_LEN 256


#define FOREGROUND 0
#define BACKGROUND 1

extern char **environ;
void parse_cmd(Job *job,char *line,int *bg){
	char **args = (char **)calloc(100,sizeof(char*));
	assert(args!=NULL);
	char *cmd = strtok(line," ");
	args[0] = (char *)calloc(strlen(cmd)+1,sizeof(char));
	strcpy(args[0],cmd);
	int i = 1;
	char *s;

	Redirection *rs[5];
	int redirect_num = 0;


	*bg = FOREGROUND;
	while((s = strtok(NULL," "))!=NULL){
		if(!strcmp(s,"&")){
			//设置后台进程
			*bg = BACKGROUND;
			continue;
		}
		if(!strcmp(s,"<")){
			char *file = strtok(NULL," ");
			if(file==NULL) continue;
			else{
				int fd = open(file,O_RDONLY);
				rs[redirect_num++]=creat_redirect(fd,RedirectRead);
			}
			continue;
		}
			
		if(!strcmp(s,">")){
			char *file = strtok(NULL," ");
			if(file==NULL) continue;
			else{
				int fd = open(file,O_WRONLY|O_CREAT|O_TRUNC,
					S_IRWXU|S_IRWXG|S_IRWXO);
				rs[redirect_num++]=creat_redirect(fd,RedirectWrite);
			    }
			continue;
		}
		if(!strcmp(s,">>")){			
			char *file = strtok(NULL," ");
			if(file==NULL) continue;
			else{
				int fd = open(file,O_WRONLY|O_CREAT|O_APPEND,
					S_IRWXU|S_IRWXG|S_IRWXO);
				rs[redirect_num++]=creat_redirect(fd,RedirectAppend);
			   }
			continue;
		}
		args[i] = (char * )calloc(strlen(s)+1,sizeof(char));
		strcpy(args[i],s);
		i++;
	}
	Program *prog = create_program(args);
	int k = 0;
	for(;k<redirect_num;k++){
		add_redirection(prog,rs[k]);
		destory_redirect(rs[k]);
	}
	add_program(job,prog);
			
	int j;
	for(j=0;j<i;j++){
		free(args[j]);
	}
	free(args);
}
void env_fun(void){
	int i = 0;
	char *env = NULL;
	while((env = environ[i])!=NULL){
		printf("%s\n",env);
		i++;
	}
}
void export_fun(Program *prog){
	if(prog->args[1]==NULL){
		fprintf(stderr,"invalid argument\n");
		return;
	}
	putenv(prog->args[1]);
}
void echo_fun(Program *prog){
	char *s = prog->args[1];
	if(s==NULL){
		fprintf(stderr,"echo: invaild argument\n");
		return;
	}
	if(s[0]=='$'){
		char *v = getenv(s+1);
		printf("%s\n",v);//输出环境变量
	}else{
		printf("%s\n",s);//原样输出
	}
}
void cd_fun(Program *prog){
	if(chdir(prog->args[1])<0){
		perror("cd error");
	}
}
void pwd_fun(Program *prog){
	char buffer[256];
	memset(buffer,0,sizeof(buffer));
	if(getcwd(buffer,sizeof(buffer))==NULL){
		perror("pwd error");
	}
	printf("%s\n",buffer);

}
void exit_fun(Program *prog){
	exit(0);
}

void execute_cmd(Job *job,int bg){
	int i;
	for(i = 0;i<job->progs_num;i++){
		if(!strcmp(job->progs[i].args[0],"cd")){
		cd_fun(&job->progs[i]);
		return;
		}
		if(!strcmp(job->progs[i].args[0],"pwd")){
		pwd_fun(&job->progs[i]);
		return;
		}
		if(!strcmp(job->progs[i].args[0],"exit")){
		exit_fun(&job->progs[i]);
		return;
		}
	
		if(!strcmp(job->progs[i].args[0],"env")){
		env_fun();
		return;
		}
		
		if(!strcmp(job->progs[i].args[0],"echo")){
		echo_fun(&job->progs[i]);
		return;
		}
		
		if(!strcmp(job->progs[i].args[0],"export")){
		export_fun(&job->progs[i]);
		return;
		}
//執行其他命令
		pid_t pid = fork();
		if(pid<0){
			perror("fork error");
		}else if(pid==0){//子進程
			//對標準輸入，標準輸出和追加輸出進行重定向
			signal(SIGTTIN,SIG_DFL);
			signal(SIGTTOU,SIG_DFL);
			signal(SIGINT,SIG_DFL);
			signal(SIGCHLD,SIG_DFL);
			signal(SIGTSTP,SIG_DFL);
			if(i==0){
				if(setpgid(getpid(),getpid())<0){//创建进程组为组长进程
					perror("setpgid error");
				}
				job->pgid = getpgid(getpid());//获得进程组的ID
			}else{//以后的进程加入到当前进程组中
				if(setpgid(getpid(),job->pgid)<0){
					perror("setpgid error");
				}
			}
			if(bg==FOREGROUND){
				tcsetpgrp(0,getpgid(getpid()));
			}
			job->progs[i].pid = getpid();
			int k  = 0;
			for(;k<job->progs[i].redirect_num;k++){
				if(job->progs[i].redirects[k].redirect==RedirectRead)			{
			//對標準輸入進行重定向
				if(dup2(job->progs[i].redirects[k]
				.fd,STDIN_FILENO)
					!=STDIN_FILENO){
					perror("dup2");
				}
			}
				
				if((job->progs[i].redirects[k].redirect==RedirectWrite)||(job->progs[i].redirects[k].redirect==RedirectAppend))			{
			//對標準輸入進行重定向
				if(dup2(job->progs[i].redirects[k]
				.fd,STDOUT_FILENO)
					!=STDOUT_FILENO){
					perror("dup2");
				}
			}

			}

			if(execvp(job->progs[i].args[0],job->progs[i].args)<0)			{
			perror("execvp error");
			exit(1);
			}
		}else{
			if(i==0){
				if(setpgid(pid,pid)<0){
					perror("setpgid error");
				}
				job->pgid = getpgid(pid);
			}else{
				if(setpgid(pid,job->pgid)<0){
					perror("setpgid error");
				}
			}
			if(bg==FOREGROUND){
				tcsetpgrp(0,job->pgid);//设置前台进程组
				waitpid(-job->pgid,NULL,WUNTRACED);
				//回收進程組中的所有子進程
			}
			if(bg==BACKGROUND){
				waitpid(-job->pgid,NULL,WNOHANG);
				//非阻塞版本後臺運行
			}
			/*
			if(waitpid(pid,NULL,WUNTRACED)<0){
				perror("waitpid error");
			}*/
		}
	}
}
void sig_handler(int signo){
	if(signo==SIGCHLD){
		waitpid(-1,NULL,WNOHANG);//回收进程组中的所有的子进程
	}
}
int main(void){
	
	setpgid(getpid(),getpid());//将进程加入到进程组中
	signal(SIGTTIN,SIG_IGN);
	signal(SIGTTOU,SIG_IGN);
	signal(SIGINT,SIG_IGN);
	signal(SIGTSTP,SIG_IGN);
	signal(SIGCHLD,sig_handler);//用于子线程的非阻塞的等待
	char buffer[MAX_COMMAND_LEN];
	memset(buffer,0,MAX_COMMAND_LEN);
	ssize_t size = strlen(prompt)*sizeof(char);
	write(STDOUT_FILENO,prompt,size);
	ssize_t len;
	
	int bg;//设置前台和后台进程的标志
	while(1){
		len = read(STDIN_FILENO,buffer,MAX_COMMAND_LEN);
		//从标准输入读取命令及其命令参数信息
		buffer[len - 1] = 0;
		if(strlen(buffer)>0){//读取到命令行信息
			Job *job = create_job(buffer);//创建命令行的作业
			parse_cmd(job,buffer,&bg);//解析命令
			execute_cmd(job,bg);//执行命令
			destory_job(job);//销毁作业
		}
		else{	
		write(STDOUT_FILENO,prompt,size);//没有读取到命令行信息，输出命令提示符
		}
		memset(buffer,0,MAX_COMMAND_LEN);//重新设置读取命令行buffer数组为0
	}
  	return 0;
}
