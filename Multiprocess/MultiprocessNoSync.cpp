#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <unordered_set>

void Print(int i)
{
	while (i-- > 0)
		printf("<%d_%d:%d>\n", getpid(), getppid(), i); //system call won't be preempted
}
int main()
{
	std::unordered_set<int> chdPidSet; //for parent process to keep track of child proccesses' pid
	int chdPid;
	for (int i = 0; i < 10; ++i)
	{
		chdPid = fork();
		if (chdPid == 0) //child
		{
			Print(1000);
			break; //stop child from continuing the for loop !!!
		}
		else //parent
		{
			chdPidSet.insert(chdPid);
		}
	}
	if (chdPid != 0) //parent
	{
		while (!chdPidSet.empty())
		{
			int waitStatus;
			int chdPid2 = wait(&waitStatus);
			if (chdPid2 == -1)
			{
				perror("Wait");
				exit(EXIT_FAILURE);
			}
			chdPidSet.erase(chdPid2);
			printf("%d_%d Wait %d: %d\n", getpid(), getppid(), chdPid2, waitStatus);
		}
	}
	return 0;
}
