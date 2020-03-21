#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

static void SIGCHLDHandler(int sig)
{
	printf("%d_%d_SIGCHLD received and waiting ... : %d\n", getpid(), getppid(), sig);
	int waitStatus;
	int chd_pid = wait(&waitStatus);
	if (chd_pid == -1)
	{
		printf("%d_%d Wait Error & Exit: %s\n", getpid(), getppid(), strerror(errno));
		exit(EXIT_FAILURE);
	}
	printf("%d_%d Wait %d: %d\n", getpid(), getppid(), chd_pid, waitStatus);
}

void ParentWaitChild()
{
	int chd_pid = fork();
	if (chd_pid == -1)
	{
		printf("%d_%d Fork Error: %s\n", getpid(), getppid(), strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (chd_pid == 0) //child
	{
		printf("Child_%d_%d_%d\n", chd_pid, getpid(), getppid());
		//when child exits but parent hasn't, child process will become <defunct>
	}
	else //parent
	{
		printf("Parent_%d_%d_%d Waiting For Child ...\n", chd_pid, getpid(), getppid());
		int waitStatus;
		int chd_pid2 = wait(&waitStatus); //block until one of its child process terminates or a signal handler interrupts the call
		if (chd_pid2 == -1)
		{
			printf("%d_%d_%d Wait Error & Exit: %s\n", chd_pid, getpid(), getppid(), strerror(errno));
			exit(EXIT_FAILURE);
		}
		printf("Parent_%d_%d_%d Wait %d: %d\n", chd_pid, getpid(), getppid(), chd_pid2, waitStatus);
		sleep(10);
	}
	printf("%d_%d_%d: Exit\n", chd_pid, getpid(), getppid());
}
void ParentCatchSIGCHLDAndWait()
{
	signal(SIGCHLD, SIGCHLDHandler);
	int chd_pid = fork();
	if (chd_pid == -1)
	{
		printf("%d_%d Fork Error: %s\n", getpid(), getppid(), strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (chd_pid == 0) //child
	{
		printf("Child_%d_%d_%d\n", chd_pid, getpid(), getppid());
		//when child exits but parent hasn't, child process will become <defunct> zombie process
	}
	else //parent
	{
		printf("Parent_%d_%d_%d\n", chd_pid, getpid(), getppid());
		sleep(10);//parent will immediately wakeup when receving the signal SIGCHLD from child and invoke SIGCHLDHandler()
	}
	printf("%d_%d_%d: Exit\n", chd_pid, getpid(), getppid());
}
void ParentCatchSIGCHLDAndIgn()
{
	signal(SIGCHLD, SIG_IGN);
	int chd_pid = fork();
	if (chd_pid == -1)
	{
		printf("%d_%d Fork Error: %s\n", getpid(), getppid(), strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (chd_pid == 0) //child
	{
		printf("Child_%d_%d_%d\n", chd_pid, getpid(), getppid());
		//when child exits but parent hasn't, child process will become <defunct> zombie process
	}
	else //parent
	{
		printf("Parent_%d_%d_%d\n", chd_pid, getpid(), getppid());
		sleep(10);//parent will continue to sleep when receving the signal SIGCHLD from child because it's SIG_IGN
	}
	printf("%d_%d_%d: Exit\n", chd_pid, getpid(), getppid());
}
void LeaveChildZombie()
{
	int chd_pid = fork();
	if (chd_pid == -1)
	{
		printf("%d_%d Fork Error: %s\n", getpid(), getppid(), strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (chd_pid == 0) //child
	{
		printf("Child_%d_%d_%d\n", chd_pid, getpid(), getppid());
		//when child exits but parent hasn't, child process will become <defunct> zombie process
	}
	else //parent
	{
		printf("Parent_%d_%d_%d\n", chd_pid, getpid(), getppid());
		sleep(10);
	}
	printf("%d_%d_%d: Exit\n", chd_pid, getpid(), getppid());
}

int main(int argc, char * argv[])
{
	if (argc != 2)
	{
		printf("fork_prevent_zombie 0/1/2/3\n0: use wait, 1: use SIGCHLD handler, 2: use SIGCHLD SIG_IGN, 3: leave child zombie\n");
		exit(EXIT_FAILURE);
	}
	if (strncmp(argv[1], "0", 1) == 0)
		ParentWaitChild();
	else if (strncmp(argv[1], "1", 1) == 0)
		ParentCatchSIGCHLDAndWait();
	else if (strncmp(argv[1], "2", 1) == 0)
		ParentCatchSIGCHLDAndIgn();
	else if (strncmp(argv[1], "3", 1) == 0)
		LeaveChildZombie();
	
	return 0;
}
