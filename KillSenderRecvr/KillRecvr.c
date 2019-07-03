#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

static void SIGUSR1Handler(int sig)
{
	printf("__SIGUSR1 received: %d\n", sig);
}
static void SIGUSR2Handler(int sig)
{
	printf("__SIGUSR2 received: %d\n", sig);
}
static void SIGINTHandler(int sig)
{
	printf("__SIGINT received or Ctrl+C pressed: %d\n", sig);
}
static void SIGABRTHandler(int sig)
{
	printf("__SIGABRT received or abort() was called: %d\n", sig);
}
static void SIGTERMHandler(int sig)
{
	printf("__SIGTERM received: %d\n", sig);//SIGTERM can be caught
}
static void SIGKILLHandler(int sig)
{
	printf("__SIGKILL received: %d\n", sig);//SIGKILL cannot be caught
}
static void SIGSEGVHandler(int sig)
{
	printf("__SIGSEGV received: %d\n", sig);
}
static void SIGCHLDHandler(int sig)
{
	printf("__SIGCHLD received: %d\n", sig);
}


int main(int argc, char * argv[])
{
	signal(SIGUSR1, SIGUSR1Handler);
	signal(SIGUSR2, SIGUSR2Handler);
	signal(SIGINT, SIGINTHandler);//ctrl+c
	signal(SIGABRT, SIGABRTHandler);//raised by abort() by the process itself, cannot be blocked. the process is terminated.
	signal(SIGTERM, SIGTERMHandler);//raised when kill is invoked, can be caught.
	signal(SIGKILL, SIGKILLHandler);//SIGKILL cannot be caught
	signal(SIGSEGV, SIGSEGVHandler);//segmentation fault, raised by the kernel to the process when illegal memory is referenced
	signal(SIGCHLD,SIGCHLDHandler);//when a child process terminates, this signal is sent to the parent.
					//upon receiving this signal, parent should execute wait() system call to read child status.


	int pid = fork();
	if (pid == 0)
	{
		printf("Child: pid=%d\n", getpid());
		exit(0);//exit child process
	}
	else
		printf("Parent: pid=%d\n", getpid());
	int waitStatus;
	int pid2 = wait(&waitStatus);
	printf("pid_%d wait: pid2_%d, %d\n", getpid(), pid2, waitStatus);

	printf("pid: %d\n", getpid());
	for (;;)
	{
		printf("1. Waiting for kill(%d,signal) from another process ...\n", getpid());
		printf("2. Enter 'abort' to abort ...\n");
		printf("3. Enter 'exit' to exit ...\n");
		printf("4. Enter 'Ctrl + C' to interrupt ...\n");
		printf("5. Enter 'raise1' to raise(SIGUSR1) ...\n");
		printf("6. Enter 'raise2' to raise(SIGUSR2) ...\n");
		printf("7. Enter 'kill %d' to terminate from another process ...\n", getpid());
		printf("8. Enter 'kill -9 %d' to terminate from another process ...\n", getpid());
		char buf[128] = {'\0'};
		scanf("%s", buf);
		if (strncmp(buf, "abort", 5) == 0)
			abort();
		else if (strncmp(buf, "exit", 4) == 0)
			break;
		else if (strncmp(buf, "raise1", 6) == 0)
			raise(SIGUSR1);
		else if (strncmp(buf, "raise2", 6) == 0)
			raise(SIGUSR2);
	}
	return 0;
}
