#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int CreateAndWriteSharedMemory(char * mmapKey, char * val, unsigned int size);

int main(int argc, char * argv[])
{
	char * mmapKey = "/testSHM";
	char * msg = "This is the text to be saved and shared in shared memory in kernel space !";
	int createSize = CreateAndWriteSharedMemory(mmapKey, msg, strlen(msg));
	if (createSize == -1)
	{
		printf("CreateAndWriteSharedMemory error\n");
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}
