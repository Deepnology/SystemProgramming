#include <stdio.h>
#include <memory.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

int CreateAndWriteSharedMemory(char * mmapKey, char * val, unsigned int size)
{
	int shmFD = shm_open(mmapKey, O_CREAT | O_RDWR | O_TRUNC, 0660);
	if (shmFD == -1)
	{
		perror("SHMOpen");
		return -1;
	}
	int truncStatus = ftruncate(shmFD, size);//resize the shared memory object in kernel space
	if (truncStatus == -1)
	{
		perror("FTruncate");
		return -1;
	}
	void * shmPtr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shmFD, 0);
	//map process's virtual address space to the shared memory object in kernel space from begin with length=size, and return ptr to process's virtual address space
	if (shmPtr == MAP_FAILED)
	{
		perror("Mmap");
		return -1;
	}
	memset(shmPtr, 0, size);
	memcpy(shmPtr, val, size);
	int unmapStatus = munmap(shmPtr, size);//destroy the mapping
	if (unmapStatus == -1)
	{
		perror("Munmap");
		return -1;
	}
	int closeStatus = close(shmFD);//deference the fd
	if (closeStatus == -1)
	{
		perror("Close");
		return -1;
	}
	return size;
}
int ReadFromSharedMemory(char * mmapKey, char * buff, unsigned int buffSize, unsigned int bytesRead)
{
	int shmFD = shm_open(mmapKey, O_CREAT | O_RDONLY, 0660);
	if (shmFD == -1)
	{
		perror("SHMOpen");
		return -1;
	}
	void * shmPtr = mmap(NULL, bytesRead, PROT_READ, MAP_SHARED, shmFD, 0);
	if (shmPtr == MAP_FAILED)
	{
		perror("Mmap");
		return -1;
	}
	memcpy(buff, shmPtr, bytesRead);
	int unmapStatus = munmap(shmPtr, bytesRead);
	if (unmapStatus == -1)
	{
		perror("Munmap");
		return -1;
	}
	int unlinkStatus = shm_unlink(mmapKey);
	if (unlinkStatus == -1)
	{
		perror("Unlink");
		return -1;
	}
	int closeStatus = close(shmFD);
	if (closeStatus == -1)
	{
		perror("Close");
		return -1;
	}
	return bytesRead;
}
