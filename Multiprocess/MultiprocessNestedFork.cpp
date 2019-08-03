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

#define CHILD_COUNT 10
#define PRINT_COUNT 500
#define APPROACH 2

typedef struct PidSet_
{
	int pidArr[4096];
	int nxt;
	void Insert(int pid)//member functions don't count for sizeof(PidSet) !!!
	{
		if (nxt < 4096)
			pidArr[nxt++] = pid;
	}
	void Print()
	{
		printf("Total pid (%d):\n", nxt);
		for (int i = 0; i < nxt; ++i)
			printf("%d,", pidArr[i]);
		printf("\n");
	}
} PidSet;

void Print(int level, int i)
{	
	printf("<%d_%d_%d:%d>--------vvvvvvvv---------\n", level, getpid(), getppid(), i);
	while (i-- > 0)
		printf("<%d_%d_%d:%d>\n", level, getpid(), getppid(), i);
	printf("<%d_%d_%d:%d>--------^^^^^^^^---------\n", level, getpid(), getppid(), i);
}
int main()
{

	const char * semMmapKey = "/SHM_SEM";
	int semShmFD = shm_open(semMmapKey, O_CREAT | O_RDWR | O_TRUNC, 0660);
	printf("Parent_%d_%d ShmOpenSem: %d\n", getpid(), getppid(), semShmFD);
	if (semShmFD == -1)
	{
		perror("SHMOpenSem");
		exit(EXIT_FAILURE);
	}
	const char * setMmapKey = "/SHM_SET";
	int setShmFD = shm_open(setMmapKey, O_CREAT | O_RDWR | O_TRUNC, 0660);
	printf("Parent_%d_%d ShmOpenSet: %d\n", getpid(), getppid(), setShmFD);
	if (setShmFD == -1)
	{
		perror("SHMOpenSet");
		exit(EXIT_FAILURE);
	}

	std::size_t sizeSem = sizeof(sem_t);
	int truncStatus = ftruncate(semShmFD, sizeSem); //resize the shared memory object in kernel space
	printf("Parent_%d_%d FTruncateSem: status=%d, size=%d\n", getpid(), getppid(), truncStatus, sizeSem);
	if (truncStatus == -1)
	{
		perror("FTruncateSem");
		exit(EXIT_FAILURE);
	}
	std::size_t sizeSet = sizeof(PidSet);
	truncStatus = ftruncate(setShmFD, sizeSet); //resize the shared memory object in kernel space
	printf("Parent_%d_%d FTruncateSet: status=%d, size=%d\n", getpid(), getppid(), truncStatus, sizeSet);
	if (truncStatus == -1)
	{
		perror("FTruncateSet");
		exit(EXIT_FAILURE);
	}

	//map process's virtual address space to the shared memory object in kernel space from begin with length=sizeSem, and return ptr to process's virtual address space
	void * semShmPtr = mmap(NULL, sizeSem, PROT_READ | PROT_WRITE, MAP_SHARED, semShmFD, 0);
	printf("Parent_%d_%d MmapSem: %p\n", getpid(), getppid(), semShmPtr);
	if (semShmPtr == MAP_FAILED)
	{
		perror("MmapSem");
		exit(EXIT_FAILURE);
	}
	memset(semShmPtr, 0, sizeSem);
	void * setShmPtr = mmap(NULL, sizeSet, PROT_READ | PROT_WRITE, MAP_SHARED, setShmFD, 0);
	printf("Parent_%d_%d MmapSet: %p\n", getpid(), getppid(), setShmPtr);
	if (setShmPtr == MAP_FAILED)
	{
		perror("MmapSet");
		exit(EXIT_FAILURE);
	}
	memset(setShmPtr, 0, sizeSet);

#if APPROACH == 1

	//approach 1: create a local semaphore then copy it into shared memory
	sem_t semaphoreTmp;
	int semInit = sem_init(&semaphoreTmp, 1, 1); //pshared != 0: can be shared between processes (and should be placed in shared memory); value = 1: binary semaphore and is equivalent to mutex
	int semVal;
	sem_getvalue(&semaphoreTmp, &semVal);
	printf("Parent_%d_%d SemInit: %d (init val = %d) Approach 1\n", getpid(), getppid(), semInit, semVal);
	if (semInit == -1)
	{
		perror("SemInit");
		exit(EXIT_FAILURE);
	}
	memcpy(semShmPtr, &semaphoreTmp, sizeSem); //copy the semaphore into shared memory by parent process
	sem_t * semaphore = (sem_t*)semShmPtr;
	//then use the copied semaphore in shared memory (discarding the one in vritual memory)
	PidSet pidSetTmp;
	memset(&pidSetTmp, 0, sizeSet);
	memcpy(setShmPtr, &pidSetTmp, sizeSet);
	PidSet * pidSet = (PidSet*)setShmPtr;
	
#elif APPROACH == 2

	//approach 2: use placement new
	sem_t * semaphore = new (semShmPtr) sem_t;
	int semInit = sem_init(semaphore, 1, 1);
	int semVal;
	sem_getvalue(semaphore, &semVal);
	printf("Parent_%d_%d SemInit: %d (init val = %d) Approach 2\n", getpid(), getppid(), semInit, semVal);
	if (semInit == -1)
	{
		perror("SemInit");
		exit(EXIT_FAILURE);
	}
	PidSet * pidSet = new (setShmPtr) PidSet;
	
#endif

	// child: fork; parent: don't fork
	//=== Level1 ===
	std::unordered_set<int> chdPidSet;
	int chdPid;
	for (int i = 0; i < CHILD_COUNT; ++i)
	{
		chdPid = fork(); //child process will begin from here !!!
		//printf("1=>%d_%d\n", getpid(), getppid());
		if (chdPid == 0) //child of Level1
		{
			//=== Level2 === (chdPid==0)
			std::unordered_set<int> chdPidSet2;
			int chdPid2;
			for (int j = 0; j < CHILD_COUNT; ++j)
			{
				chdPid2 = fork();
				//printf("2=>%d_%d\n", getpid(), getppid());
				if (chdPid2 == 0) //child of Level2
				{
					//=== Level3 === (chdPid==0, chdPid2==0)
					std::unordered_set<int> chdPidSet3;
					int chdPid3;
					for (int k = 0; k < CHILD_COUNT; ++k)
					{
						chdPid3 = fork();
						//printf("3=>%d_%d\n", getpid(), getppid());
						if (chdPid3 == 0) //child of Level3
						{
							sem_wait(semaphore);
							pidSet->Insert(getpid());
							Print(3, PRINT_COUNT); //critical section: child of Level3
							sem_post(semaphore);
							break;//exit for loop of Level3 for child of Level3
						}
						else //parent of Level3
						{
							chdPidSet3.insert(chdPid3);
						}
					}//end for loop Level3
					if (chdPid3 != 0)
					{
						sem_wait(semaphore);
						pidSet->Insert(getpid());
						Print(2, PRINT_COUNT); //critical section: child of Level2 (parent of Level3)
						sem_post(semaphore);

						while (!chdPidSet3.empty())
						{
							int waitStatus;
							int chdPid33 = wait(&waitStatus);
							if (chdPid33 == -1)
							{
								perror("Wait");
								exit(EXIT_FAILURE);
							}
							chdPidSet3.erase(chdPid33);
							printf("%d_%d Wait %d: %d\n", getpid(), getppid(), chdPid33, waitStatus);
						}
					}
					//=== Level3 End === (chdPid==0, chdPid2==0)
					break;//exit for loop of Level2 for child of Level2
				}
				else //parent of Level2
				{
					chdPidSet2.insert(chdPid2);
				}
			}//end for loop Level2
			
			if (chdPid2 != 0) //parent of Level2
			{
				sem_wait(semaphore);
				pidSet->Insert(getpid());
				Print(1, PRINT_COUNT); //critical section: child of Level1 ( parent of Level2)
				sem_post(semaphore);

				while (!chdPidSet2.empty())
				{
					int waitStatus;
					int chdPid22 = wait(&waitStatus);
					if (chdPid22 == -1)
					{
						perror("Wait");
						exit(EXIT_FAILURE);
					}
					chdPidSet2.erase(chdPid22);
					printf("%d_%d Wait %d: %d\n", getpid(), getppid(), chdPid22, waitStatus);
				}
			}
			//=== Level2 End === (chdPid==0)
			break;//exit for loop of Level1 for child of Level1
		}
		else //parent of Level1
		{
			chdPidSet.insert(chdPid);
		}
	}//end of for loop Level1

	if (chdPid != 0) //parent of Level1
	{
		sem_wait(semaphore);
		pidSet->Insert(getpid());
		Print(0, PRINT_COUNT); //critical section: child of Level0 (parent of Level1)
		sem_post(semaphore);

		while (!chdPidSet.empty())
		{
			int waitStatus;
			int chdPid00 = wait(&waitStatus);
			if (chdPid00 == -1)
			{
				perror("Wait");
				exit(EXIT_FAILURE);
			}
			chdPidSet.erase(chdPid00);
			printf("%d_%d Wait %d: %d\n", getpid(), getppid(), chdPid00, waitStatus);
		}
	}
	//=== Level1 End ===

#if APPROACH == 2
	//delete semaphore; //no need to delete !!!
	//the semaphore variable points to memory chunk which is still a mapped memory, and really do not reside on heap.
	//Any memory which do not reside on heap cannot be deleted/freed.
#endif


	if (chdPid != 0)
	{
		//total process count = ChildCount^3 + ChildCount^2 + ChildCount^1 + ChildCount^0
		pidSet->Print();
	}

	int unmapStatus = munmap(semShmPtr, sizeSem); //destroy the mapping
	printf("%d_%d MunmapSem: %d\n", getpid(), getppid(), unmapStatus);
	if (unmapStatus == -1)
	{
		perror("MunmapSem");
		exit(EXIT_FAILURE);
	}
	unmapStatus = munmap(setShmPtr, sizeSet); //destroy the mapping
	printf("%d_%d MunmapSet: %d\n", getpid(), getppid(), unmapStatus);
	if (unmapStatus == -1)
	{
		perror("MunmapSet");
		exit(EXIT_FAILURE);
	}
	int closeStatus = close(semShmFD); //deference the fd
	printf("%d_%d CloseSem: %d\n", getpid(), getppid(), closeStatus);
	if (closeStatus == -1)
	{
		perror("CloseSem");
		exit(EXIT_FAILURE);
	}
	closeStatus = close(setShmFD); //deference the fd
	printf("%d_%d CloseSet: %d\n", getpid(), getppid(), closeStatus);
	if (closeStatus == -1)
	{
		perror("CloseSet");
		exit(EXIT_FAILURE);
	}

	if (chdPid != 0)
	{
		int unlinkStatus = shm_unlink(semMmapKey); //unlink shm by parent process only !!!
		printf("%d_%d UnlinkSem: %d\n", getpid(), getppid(), unlinkStatus);
		if (unlinkStatus == -1)
		{
			perror("UnlinkSem");
			exit(EXIT_FAILURE);
		}
		unlinkStatus = shm_unlink(setMmapKey); //unlink shm by parent process only !!!
		printf("%d_%d UnlinkSet: %d\n", getpid(), getppid(), unlinkStatus);
		if (unlinkStatus == -1)
		{
			perror("UnlinkSet");
			exit(EXIT_FAILURE);
		}
	}
	return 0;
}

/*
g++ MultiprocessNestedFork.cpp -o MultiprocessNestedFork -lpthread -lrt

valgrind --leak-check=full MultiprocessNestedFork
*/
