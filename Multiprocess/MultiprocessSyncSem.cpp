#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <unordered_set>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>

#define APPROACH 2
void Print(int i)
{
	
	printf("<%d_%d:%d>--------vvvvvvvv---------\n", getpid(), getppid(), i);
	while (i-- > 0)
		printf("<%d_%d:%d>\n", getpid(), getppid(), i);
	printf("<%d_%d:%d>--------^^^^^^^^---------\n", getpid(), getppid(), i);
}
int main()
{

	const char * mmapKey = "/testSHM";
	int shmFD = shm_open(mmapKey, O_CREAT | O_RDWR | O_TRUNC, 0660);
	printf("Parent_%d_%d ShmOpen: %d\n", getpid(), getppid(), shmFD);
	if (shmFD == -1)
	{
		perror("SHMOpen");
		exit(EXIT_FAILURE);
	}
	std::size_t sizeSem = sizeof(sem_t);
	int truncStatus = ftruncate(shmFD, sizeSem); //resize the shared memory object in kernel space
	printf("Parent_%d_%d FTruncate: %d\n", getpid(), getppid(), truncStatus);
	if (truncStatus == -1)
	{
		perror("FTruncate");
		exit(EXIT_FAILURE);
	}
	//map process's virtual address space to the shared memory object in kernel space from begin with length=sizeSem, and return ptr to process's virtual address space
	void * shmPtr = mmap(NULL, sizeSem, PROT_READ | PROT_WRITE, MAP_SHARED, shmFD, 0);
	printf("Parent_%d_%d Mmap: %p\n", getpid(), getppid(), shmPtr);
	if (shmPtr == MAP_FAILED)
	{
		perror("Mmap");
		exit(EXIT_FAILURE);
	}
	memset(shmPtr, 0, sizeSem);

#if APPROACH == 1

	//approach 1: create a local semaphore then copy it into shared memory
	sem_t semaphore;
	int semInit = sem_init(&semaphore, 1, 1); //pshared != 0: can be shared between processes (and should be placed in shared memory); value = 1: binary semaphore and is equivalent to mutex
	int semVal;
	sem_getvalue(&semaphore, &semVal);
	printf("Parent_%d_%d SemInit: %d (init val = %d) Approach 1\n", getpid(), getppid(), semInit, semVal);
	if (semInit == -1)
	{
		perror("SemInit");
		exit(EXIT_FAILURE);
	}
	memcpy(shmPtr, &semaphore, sizeSem); //copy the semaphore into shared memory by parent process
	//then use the copied semaphore in shared memory (discarding the one in vritual memory)
	
#elif APPROACH == 2

	//approach 2: use placement new
	sem_t * semaphore = new (shmPtr) sem_t;
	int semInit = sem_init(semaphore, 1, 1);
	int semVal;
	sem_getvalue(semaphore, &semVal);
	printf("Parent_%d_%d SemInit: %d (init val = %d) Approach 2\n", getpid(), getppid(), semInit, semVal);
	if (semInit == -1)
	{
		perror("SemInit");
		exit(EXIT_FAILURE);
	}

#endif


	std::unordered_set<int> chdPidSet;
	int chdPid;
	for (int i = 0; i < 10; ++i)
	{
		chdPid = fork(); //child process will begin from here !!!
		if (chdPid == 0) //child
		{
			sem_wait((sem_t*)shmPtr);
			Print(1000); //critical section
			sem_post((sem_t*)shmPtr);
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


#if APPROACH == 2
	//delete semaphore; //no need to delete !!!
	//the semaphore variable points to memory chunk which is still a mapped memory, and really do not reside on heap.
	//Any memory which do not reside on heap cannot be deleted/freed.
#endif


	int unmapStatus = munmap(shmPtr, sizeSem); //destroy the mapping
	printf("%d_%d Munmap: %d\n", getpid(), getppid(), unmapStatus);
	if (unmapStatus == -1)
	{
		perror("Munmap");
		exit(EXIT_FAILURE);
	}
	int closeStatus = close(shmFD); //deference the fd
	printf("%d_%d Close: %d\n", getpid(), getppid(), closeStatus);
	if (closeStatus == -1)
	{
		perror("Close");
		exit(EXIT_FAILURE);
	}

	if (chdPid != 0)
	{
		int unlinkStatus = shm_unlink(mmapKey); //unlink shm by parent process only !!!
		printf("%d_%d Unlink: %d\n", getpid(), getppid(), unlinkStatus);
		if (unlinkStatus == -1)
		{
			perror("Unlink");
			exit(EXIT_FAILURE);
		}
	}
	return 0;
}

/*
g++ MultiprocessSyncSem.cpp -o MultiprocessSyncSem -lpthread -lrt

valgrind --leak-check=full MultiprocessSyncSem
*/
