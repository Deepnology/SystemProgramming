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

#define BUFFER_SIZE 30
#define MAX_PROCESS_COUNT 1024

struct ProducerConsumerQueue
{
	sem_t m_semaphoreMutex;
	sem_t m_semaphoreProduce;
	sem_t m_semaphoreConsume;
	std::size_t m_head;
	std::size_t m_tail;
	std::size_t m_count;
	int m_buff[BUFFER_SIZE];

	std::size_t m_next;
	int m_pid[MAX_PROCESS_COUNT];

	ProducerConsumerQueue(int numProducer)
	{
		if (numProducer > BUFFER_SIZE)
		{
			printf("Error: numProducer > BUFFER_SIZE\n");
			exit(EXIT_FAILURE);
		}
		int semInit = sem_init(&m_semaphoreMutex, 1, 1);
		if (semInit == -1)
		{
			perror("SemInit");
			exit(EXIT_FAILURE);
		}
		semInit = sem_init(&m_semaphoreProduce, 1, numProducer);
		if (semInit == -1)
		{
			perror("SemInit");
			exit(EXIT_FAILURE);
		}
		semInit = sem_init(&m_semaphoreConsume, 1, 0);
		if (semInit == -1)
		{
			perror("SemInit");
			exit(EXIT_FAILURE);
		}

		int semVal;
		sem_getvalue(&m_semaphoreMutex, &semVal);
		printf("ProducerConsumerQueue::m_semaphoreMutex: %d\n", semVal);
		sem_getvalue(&m_semaphoreProduce, &semVal);
		printf("ProducerConsumerQueue::m_semaphoreProduce: %d\n", semVal);
		sem_getvalue(&m_semaphoreConsume, &semVal);
		printf("ProducerConsumerQueue::m_semaphoreConsume: %d\n", semVal);

		m_head = 0;
		m_tail = 0;
		m_count = 0;
		memset(m_buff, 0, BUFFER_SIZE);

		m_next = 0;
		memset(m_pid, 0, MAX_PROCESS_COUNT);
	}

	void Produce(int i)
	{
		sem_wait(&m_semaphoreProduce);//numProducers
		{
			sem_wait(&m_semaphoreMutex);//1
			{
				if (m_count == BUFFER_SIZE)
				{
					printf("<%d_%d_F>", getpid(), getppid());
				}
				else
				{
					m_buff[m_tail] = i;
					m_tail = (m_tail+1) % BUFFER_SIZE;
					++m_count;
					printf("<%d_%d_%d>", getpid(), getppid(), m_count);
				}
			}
			sem_post(&m_semaphoreMutex);
		}
		sem_post(&m_semaphoreConsume);
	}
	int Consume()
	{
		int res;
		sem_wait(&m_semaphoreConsume);//0
		{
			sem_wait(&m_semaphoreMutex);//1
			{
				if (m_count == 0)
				{
					printf("[%d_%d_E]", getpid(), getppid());
					res = -1;
				}
				else
				{
					--m_count;
					res = m_buff[m_head];
					m_head = (m_head+1) % BUFFER_SIZE;
					printf("[%d_%d_%d]", getpid(), getppid(), m_count);
				}
			}
			sem_post(&m_semaphoreMutex);
		}
		sem_post(&m_semaphoreProduce);
		return res;
	}

	void Insert(int pid)
	{
		sem_wait(&m_semaphoreMutex);
		if (m_next < MAX_PROCESS_COUNT)
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
	std::size_t sizePCQ = sizeof(ProducerConsumerQueue);
	int truncStatus = ftruncate(shmFD, sizePCQ); //resize the shared memory object in kernel space
	printf("Parent_%d_%d FTruncate: status=%d, size=%d\n", getpid(), getppid(), truncStatus, sizePCQ);
	if (truncStatus == -1)
	{
		perror("FTruncate");
		exit(EXIT_FAILURE);
	}
	//map process's virtual address space to the shared memory object in kernel space from begin with length=sizeSem, and return ptr to process's virtual address space
	void * shmPtr = mmap(NULL, sizePCQ, PROT_READ | PROT_WRITE, MAP_SHARED, shmFD, 0);
	printf("Parent_%d_%d Mmap: %p\n", getpid(), getppid(), shmPtr);
	if (shmPtr == MAP_FAILED)
	{
		perror("Mmap");
		exit(EXIT_FAILURE);
	}
	memset(shmPtr, 0, sizePCQ);

	ProducerConsumerQueue * pcq = new (shmPtr) ProducerConsumerQueue(BUFFER_SIZE); //let max concurrent producers equal to BUFFER_SIZE
	printf("Parent_%d_%d Create ProducerConsumerQueue object\n", getpid(), getppid());
	
	std::unordered_set<int> chdPidSet;
	int chdPid;
	for (int i = 0; i < 40; ++i)
	{
		chdPid = fork(); //child process will begin from here !!!
		if (chdPid == 0) //child
		{
			pcq->Insert(getpid());
			int j = 10000;
			while (j-->0)
			{
				if (i % 2 == 0)//when producerCount==consumerCount, there will be no deadlock for the while loop
					pcq->Consume();//20 consumer processes
				else
					pcq->Produce(getpid());//20 producer processes
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
		pcq->PrintPid();
	}

	int unmapStatus = munmap(shmPtr, sizePCQ); //destroy the mapping
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
