#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#define TOK_BUFSIZE 64
#define ARG_BUFSIZE 1000
#define ARR_SIZE 100
#define TOK_DELIM " \t\r\n\a"
int count = 0;
int position;
typedef struct 
{
    char argbuff[ARG_BUFSIZE];
    char **args;
} sargs;

typedef struct 
{
    char *name;
    pid_t pid;
} pid_s;

typedef struct 
{
    char *key;
    char *value;
} env;

pid_s pid[ARR_SIZE];

char **split_line(char *line)
{
	int bufsize = TOK_BUFSIZE;
	position = 0;
	char **tokens = malloc(bufsize * sizeof(char*));
	char *token;

	if (!tokens) 
	{
		perror("allocation error\n");
		return 0;
	}

	token = strtok(line, TOK_DELIM);
	while (token != NULL) 
	{
		tokens[position] = token;
		position++;

		if (position >= bufsize) 
		{
			bufsize += TOK_BUFSIZE;
			tokens = realloc(tokens, bufsize * sizeof(char*));
			if (!tokens) 
			{
				perror("allocation error\n");
				return 0;
			}
		}

		token = strtok(NULL, TOK_DELIM);
	}
	tokens[position] = NULL;
	return tokens;
}

void exe(sargs *a)
{
	if (execve(a->args[0], a->args, NULL) < 0)
	{
		perror("execve");
		exit(1);
	}
			

}
void taskremove(pid_t pidt)
{
	for(int i = 0; i< count;i++)
	{	
		if (pidt == pid[i].pid)
		{
			free(pid[i].name);
			 for (int c = i; c < count; c++)
					pid[c].pid = pid[c+1].pid;
			count--;
		}
	}
}

void handler(int sig)
{
	pid_t pidt;

	if ((pidt = waitpid(-1, NULL, WNOHANG)) != -1)
		taskremove(pidt);
}

int statuspick(sargs argsS,int status)
{
	
		if(strstr(argsS.args[0],"tasks"))
			return status = 2;
		else if(strstr(argsS.args[0],"return"))
			return status = 3;
		else if(strstr(argsS.args[0],"="))
			return status = 5;
		else if(strstr(argsS.args[0],"print_env"))
			return status = 6;

		if(position > 1)
		{
			if (strstr(argsS.args[position -1],"&"))
			return status = 1;
		}
		if(position >= 3)
		{
			if(strstr(argsS.args[position - 2],">"))
				return status = 4;
			else if(strstr(argsS.args[1],"<"))
				return status = 7;
		}
		return status = 0; 

}


int main (int argc, const char* argv[])
{
	pthread_t tid1;
	sargs argsS;
	env env_a[ARR_SIZE];
  	int countenv, freeflag = 0;
	int status, i;
	int envflag;
	while(1)
	{
		if(freeflag)
			free(argsS.args);
		freeflag = 1;
		status, envflag = 0;
		pid_t pids;
		printf("> ");
		fgets(argsS.argbuff,ARG_BUFSIZE,stdin);
		strtok(argsS.argbuff, "\n");
		argsS.args = split_line(argsS.argbuff);
		for ( int i = 0; i < position; i++)
			for (int j = 0 ; j < countenv ; j++)
				if(!(strcmp(argsS.args[i],env_a[j].key)))
					argsS.args[i] = strdup(	env_a[j].value);
		status = statuspick(argsS,status);				
		signal(SIGCHLD, handler);

		switch(status)
		{
			case 0:
			{
				signal(SIGCHLD, SIG_DFL);
				if ((pids = fork()) < 0)
					perror("fork");
				else if (!pids)
					exe(&argsS);
				else
				{
					int status;
					if ((pids = waitpid(pids, &status, 0) < 0))
						perror("waitpid");
				}
				break;
			}
		

			
			case 1:
			{	
				if((strcmp(argsS.args[position-1],"&")) | position == 1)
				{
					printf("misplace arg\n");
					break;
				}
				argsS.args[position-1] = NULL;
				pid[count].name = strdup(argsS.args[0]);
				if ((pid[count].pid = fork()) < 0)
				{
					perror("fork");
					free(pid[count].name);
				}
				else if (!pid[count].pid)
					exe(&argsS);
				else
					count++;
				break;
			}
			
			case 2:
			{
				for (int i = 0; i< count ; i++)
					printf("%d:%s\n",pid[i].pid, pid[i].name);	
				break;
			}
			
			case 3:
			{
				signal(SIGCHLD, SIG_DFL);
				pid_t tempid, savet;
				tempid = atoi(argsS.args[1]);
				for(int i = 0; i< count;i++)
				{
					if (tempid == pid[i].pid)
					{
						savet = tempid;
						int status;
						if ((tempid = waitpid(tempid, &status, 0) < 0))
							perror("waitpid");
						taskremove(savet);
						break;
					}
				
				}
				break;
			}
			case 4:
			{
				if(!(strcmp(argsS.args[position-1],">")) | !(strcmp(argsS.args[0],">")) )
				{
					printf("not enough args\n");
					break;
				}
				
				signal(SIGCHLD, SIG_DFL);
				int out_fd; 
				out_fd = open(argsS.args[position-1], O_WRONLY | O_CREAT,0666);
				if (out_fd == -1)
				{
					perror("open");
					return 1;
				}
				argsS.args[position-1] = NULL;
				argsS.args[position-2] = NULL;
				if ((pids = fork()) < 0)
					perror("fork");
				else if (!pids)
				{
			
					if (close(STDOUT_FILENO) < 0)
						perror("close");
					dup(out_fd);
					exe(&argsS);
					if (out_fd < 0)
						perror("open");
					if (out_fd != STDOUT_FILENO)
						return 1;
				}
				else
				{
					int status;
					if ((pids = waitpid(pids, &status, 0) < 0))
						perror("waitpid");
				}
				break;
			}
			case 5:
			{
				char * tok;
				int flag = 1;
				char string[ARG_BUFSIZE];
				if(position > 1 )
				{
					printf("too many args\n");
					break;
				}
				tok = strtok(argsS.args[0],"=");
				for( int i = 0; i< countenv; i++)
				{
					if(!(strcmp(env_a[i].key,tok)))
					{
						printf("value is already in use\n");
						flag = 0;
						break;
					}

				}
				if(flag)
				{
					env_a[countenv].key = strdup(tok);
					tok = strtok(NULL, "");
					env_a[countenv].value = strdup(tok);
					countenv++;
				}
				break;
			}
			case 6:
			{
				for(int i = 0; i< countenv; i++)
					printf("%s:%s\n",env_a[i].key, env_a[i].value);
				break;
			}
			case 7:
			{
				if(!(strcmp(argsS.args[position-1],"<")) | !(strcmp(argsS.args[0],"<")) )
				{
					printf("not enough args\n");
					break;
				}
				
				signal(SIGCHLD, SIG_DFL);
				int in_fd;
				in_fd = open(argsS.args[position - 1], O_RDONLY,0666);
				if (in_fd == -1)
				{
					perror("open");
					return 1;
				}
				if ((pids = fork()) < 0)
					perror("fork");
				else if (!pids)
				{
			
					if (close(STDIN_FILENO) < 0)
						perror("close");
					dup(in_fd);
					exe(&argsS);
					if (in_fd < 0)
						perror("open");
					if (in_fd != STDIN_FILENO)
						return 1;
				}
				else
				{
					int status;
					if ((pids = waitpid(pids, &status, 0) < 0))
						perror("waitpid");
				}
				break;
			}
		}
	}
	return 0;
}