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

#define BUFFER_SIZE 1024
#define MAX_PROCESS_COUNT 1024

struct DiningPhilosophers
{
	sem_t m_semaphoreMutex;
	std::size_t m_count;
	sem_t m_buff[BUFFER_SIZE];

	std::size_t m_next;
	int m_pid[MAX_PROCESS_COUNT];

	DiningPhilosophers(int numPhilosopher)
	{
		if (numPhilosopher > BUFFER_SIZE)
		{
			printf("Error: numPhilosopher > BUFFER_SIZE\n");
			exit(EXIT_FAILURE);
		}
		int semInit = sem_init(&m_semaphoreMutex, 1, 1);
		if (semInit == -1)
		{
			perror("SemInit");
			exit(EXIT_FAILURE);
		}

		m_count = numPhilosopher;
		for (int i = 0; i < m_count; ++i)
		{
			semInit = sem_init(&m_buff[i], 1, 1);
			if (semInit == -1)
			{
				perror("SemInit");
				exit(EXIT_FAILURE);
			}
		}

		int semVal;
		sem_getvalue(&m_semaphoreMutex, &semVal);
		printf("DiningPhilosophers::m_semaphoreMutex: %d\n", semVal);
		for (int i = 0; i < m_count; ++i)
		{
			sem_getvalue(&m_buff[i], &semVal);
			printf("DiningPhilosophers::m_buff[%d]: %d\n", i, semVal);
		}

		m_next = 0;
		memset(m_pid, 0, MAX_PROCESS_COUNT);
	}

	void Philosopher(int id)
	{
		int low = id;
		int high = (id+1) % m_count;
		if (low > high)
		{
			int tmp = low;
			low = high;
			high = tmp;
		}
		sem_wait(&m_buff[low]);
		sem_wait(&m_buff[high]);

		long microSec = (rand() % 1000 + 1) * 10000;//1 microsec = 1000 nanosec
		timespec sleepVal = {0};
		sleepVal.tv_nsec = microSec;
		nanosleep(&sleepVal, NULL);

		printf("<%d_%d_%d>", id, getpid(), getppid());

		sem_post(&m_buff[low]);
		sem_post(&m_buff[high]);
	}

	void Insert(int pid)
	{
		sem_wait(&m_semaphoreMutex);
		if (m_next >= MAX_PROCESS_COUNT) return;
		m_pid[m_next++] = pid;
		sem_post(&m_semaphoreMutex);
	}
	void PrintPid()
	{
		printf("Total pid (%d):\n", m_next);
		for (int i = 0; i < m_next; ++i)
			printf("%d,", m_pid[i]);
		printf("\n");
	}
};



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
	std::size_t sizeDP = sizeof(DiningPhilosophers);
	int truncStatus = ftruncate(shmFD, sizeDP); //resize the shared memory object in kernel space
	printf("Parent_%d_%d FTruncate: status=%d, size=%d\n", getpid(), getppid(), truncStatus, sizeDP);
	if (truncStatus == -1)
	{
		perror("FTruncate");
		exit(EXIT_FAILURE);
	}
	//map process's virtual address space to the shared memory object in kernel space from begin with length=sizeSem, and return ptr to process's virtual address space
	void * shmPtr = mmap(NULL, sizeDP, PROT_READ | PROT_WRITE, MAP_SHARED, shmFD, 0);
	printf("Parent_%d_%d Mmap: %p\n", getpid(), getppid(), shmPtr);
	if (shmPtr == MAP_FAILED)
	{
		perror("Mmap");
		exit(EXIT_FAILURE);
	}
	memset(shmPtr, 0, sizeDP);

	DiningPhilosophers * dp = new (shmPtr) DiningPhilosophers(40);
	printf("Parent_%d_%d Create DiningPhilosophers object\n", getpid(), getppid());
	
	std::unordered_set<int> chdPidSet;
	int chdPid;
	for (int i = 0; i < 40; ++i)
	{
		chdPid = fork(); //child process will begin from here !!!
		if (chdPid == 0) //child
		{
			dp->Insert(getpid());
			int j = 1000;
			while (j-->0)
			{
				long microSec = (rand() % 1000 + 1) * 10000;//1 microsec = 1000 nanosec
				timespec sleepVal = {0};
				sleepVal.tv_nsec = microSec;
				nanosleep(&sleepVal, NULL);
				
				dp->Philosopher(i);
			}
			break; //stop child from continuing the for loop !!!
		}
		else //parent
		{
			chdPidSet.insert(chdPid);
		}
	}
	if (chdPid != 0) //parent
	{
		printf("Parent_%d_%d start to wait child ...\n", getpid(), getppid());
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

	if (chdPid != 0)
	{
		dp->PrintPid();
	}

	int unmapStatus = munmap(shmPtr, sizeDP); //destroy the mapping
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
g++ SyncProducersConsumers.cpp -o SyncProducersConsumers -lpthread -lrt

valgrind --leak-check=full ./SyncProducersConsumers
*/
