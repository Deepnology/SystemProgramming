#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char * argv[])
{
	for (;;)
	{
		int pid; char buf[32] = {'\0'};
		printf("1. Enter 'pid SIGUSR1/SIGUSR2/SIGINT/SIGABRT/SIGTERM/SIGKILL/SIGSTOP/SIGCONT/SIGHUP/SIGQUIT/SIGILL/SIGTRAP'\nto kill(pid,signal) from %d_%d ...\n", getpid(), getppid());
		printf("2. Enter '0' to exit ...\n");
		scanf("%d %s", &pid, buf);
		if (pid == 0) break;
		if (strncmp(buf, "SIGUSR1", 7) == 0)
			kill(pid, SIGUSR1);
		else if (strncmp(buf, "SIGUSR2", 7) == 0)
			kill(pid, SIGUSR2);
		else if (strncmp(buf, "SIGINT", 6) == 0)
			kill(pid, SIGINT);
		else if (strncmp(buf, "SIGABRT", 7) == 0)
			kill(pid, SIGABRT);
		else if (strncmp(buf, "SIGTERM", 7) == 0)
			kill(pid, SIGTERM);
		else if (strncmp(buf, "SIGKILL", 7) == 0)
			kill(pid, SIGKILL);
		else if (strncmp(buf, "SIGSTOP", 7) == 0)
			kill(pid, SIGSTOP);
		else if (strncmp(buf, "SIGCONT", 7) == 0)
			kill(pid, SIGCONT);
		else if (strncmp(buf, "SIGHUP", 6) == 0)
			kill(pid, SIGHUP);
		else if (strncmp(buf, "SIGQUIT", 7) == 0)
			kill(pid, SIGQUIT);
		else if (strncmp(buf, "SIGILL", 6) == 0)
			kill(pid, SIGILL);
		else if (strncmp(buf, "SIGTRAP", 7) == 0)
			kill(pid, SIGTRAP);
	}
	return 0;
}
