#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int ReadFromSharedMemory(char * mmapKey, char * buff, unsigned int buffSize, unsigned int bytesRead);

int main(int argc, char * argv[])
{
	char * mmapKey = "/testSHM";
	char readBuff[128];
	memset(readBuff, 0, 128);
	int readSize = ReadFromSharedMemory(mmapKey, readBuff, 128, 128);
	if (readSize == -1)
	{
		printf("ReadFromSharedMemory error\n");
		exit(EXIT_FAILURE);
	}
	printf("Read: %s\n", readBuff);
	exit(EXIT_SUCCESS);
}
