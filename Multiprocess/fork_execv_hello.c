#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
int main()
{
	
	int chd_pid = fork();
	if (chd_pid == -1)
	{
		printf("%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (chd_pid == 0)//child
	{
		printf("Child_%d_%d_%d\n", chd_pid, getpid(), getppid());
		char * arg[1] = {0};
		int execvStatus = execv("hello", arg); //execute an executable file named hello
		//execv: load program in process address space
		printf("Child_%d_%d_%d WILL NOT REACH HERE after execv !!!\n", chd_pid, getpid(), getppid());
		printf("Child_%d_%d_%d Execv hello: %d\n:", chd_pid, getpid(), getppid(), execvStatus);
		if (execvStatus == -1)
		{
			printf("%d_%d_%d_Execv Error & Exit: %s\n", chd_pid, getpid(), getppid(), strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	else//parent
	{
		printf("Parent_%d_%d_%d\n", chd_pid, getpid(), getppid());
	}

	if (chd_pid != 0)
	{
		int waitStatus;
		int chd_pid2 = wait(&waitStatus);
		if (chd_pid2 == -1)
		{
			printf("%d_%d_%d_Wait Error & Exit: %s\n", chd_pid, getpid(), getppid(), strerror(errno));
			exit(EXIT_FAILURE);
		}
		printf("%d_%d_%d Wait %d: %d\n", chd_pid, getpid(), getppid(), chd_pid2, waitStatus);
	}

	exit(0);
}
