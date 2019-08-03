#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

int GLOBAL1 = 10; //the global var's value and its logical address will be copied to child process
static int GLOBAL2 = 20; //the static global var's value and its logical address will be copied to child process

int main()
{
	printf("%d_%d: Before Fork (Child will execute starting from the fork() call)...\n", getpid(), getppid());

	int local_var = 30; //the local var's value and its logical address will be copied to child process
	pid_t chd_pid = fork();
	if (chd_pid == -1)
	{
		printf("%d_%d Fork Error: %s\n", getpid(), getppid(), strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (chd_pid > 0)//chd_pid>0: parent process, chd_pid is child's pid
	{
		printf("Parent_%d_%d_%d: GLOBAL1 = %d, %p\n", chd_pid, getpid(), getppid(), GLOBAL1, &GLOBAL1);
		printf("Parent_%d_%d_%d: static GLOBAL2 = %d, %p\n", chd_pid, getpid(), getppid(), GLOBAL2, &GLOBAL2);
		printf("Parent_%d_%d_%d: local_var = %d, %p\n", chd_pid, getpid(), getppid(), local_var, &local_var);
		GLOBAL1 = 11;
		GLOBAL2 = 21;
		local_var = 31;
		printf("Parent_%d_%d_%d: GLOBAL1 = %d, %p\n", chd_pid, getpid(), getppid(), GLOBAL1, &GLOBAL1);
		printf("Parent_%d_%d_%d: static GLOBAL2 = %d, %p\n", chd_pid, getpid(), getppid(), GLOBAL2, &GLOBAL2);
		printf("Parent_%d_%d_%d: local_var = %d, %p\n", chd_pid, getpid(), getppid(), local_var, &local_var);
		sleep(5);
		printf("Parent_%d_%d_%d: wakes up\n", chd_pid, getpid(), getppid());
	}
	else//chd_pid=0: child process
	{
		//in the child process, all global and local variables get separated
		//while text segment will remain the same
		//for opened file descriptor from parent process, it will be available in the child process
		printf("Child_%d_%d_%d: GLOBAL1 = %d, %p\n", chd_pid, getpid(), getppid(), GLOBAL1, &GLOBAL1);
		printf("Child_%d_%d_%d: static GLOBAL2 = %d, %p\n", chd_pid, getpid(), getppid(), GLOBAL2, &GLOBAL2);
		printf("Child_%d_%d_%d: local_var = %d, %p\n", chd_pid, getpid(), getppid(), local_var, &local_var);
		GLOBAL1 = 15;
		GLOBAL2 = 25;
		local_var = 35;
		printf("Child_%d_%d_%d: GLOBAL1 = %d, %p\n", chd_pid, getpid(), getppid(), GLOBAL1, &GLOBAL1);
		printf("Child_%d_%d_%d: static GLOBAL2 = %d, %p\n", chd_pid, getpid(), getppid(), GLOBAL2, &GLOBAL2);
		printf("Child_%d_%d_%d: local_var = %d, %p\n", chd_pid, getpid(), getppid(), local_var, &local_var);
		sleep(10);
		printf("Child_%d_%d_%d: wakes up\n", chd_pid, getpid(), getppid());
	}

	if (chd_pid != 0) //for parent to wait for child to wake up
	{
		int waitStatus;
		int chd_pid2 = wait(&waitStatus); //invoke wait() on child process will result in error: No Child Processes !
		if (chd_pid2 == -1)
		{
			printf("%d_%d_%d_Wait Error & Exit: %s\n", chd_pid, getpid(), getppid(), strerror(errno));
			exit(EXIT_FAILURE);
		}
		printf("%d_%d_%d Wait %d: %d\n", chd_pid, getpid(), getppid(), chd_pid2, waitStatus);
	}

	//the following code will be run by both Parent and Child
	sleep(1);
	printf("%d_%d_%d: Exit\n", chd_pid, getpid(), getppid());
	return 0;
}

//ps -ef | grep -i fork_demo  (display the pid info of the fork_demo process)
//ps -ef | more               (displays all pid info)
//echo $$                     (displays the shell pid)
//kill -9 pid                 (kill a process)
//when the parent process gets killed, the parent process of the child process becomes 1(init process)
//when the child process gets killed or eixts before the parent, it will remain running as <defunct> (zombie process) to wait for parent process exit
