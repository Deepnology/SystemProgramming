#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SOCKET_NAME "/tmp/DemoUnixSocket"
#define BUFFER_SIZE 128

int main(int argc, char * argv[])
{
	char readBuff[BUFFER_SIZE];
	int dataBuff;

	int clientSocketFD = socket(AF_UNIX, SOCK_STREAM, 0);
	printf("Socket: %d\n", clientSocketFD);
	if (clientSocketFD == -1)
	{
		perror("Socket");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_un clntAddr;
	memset(&clntAddr, 0, sizeof(struct sockaddr_un));
	clntAddr.sun_family = AF_UNIX;
	strncpy(clntAddr.sun_path, SOCKET_NAME, sizeof(clntAddr.sun_path)-1);

	int connectStatus = connect(clientSocketFD, (struct sockaddr*) &clntAddr, sizeof(struct sockaddr_un));
	printf("Connect: %d\n", connectStatus);
	if (connectStatus == -1)
	{
		perror("Connect");
		exit(EXIT_FAILURE);
	}

	for(;;)
	{
		printf("1. Enter number to be accumulated in server: \n");
		printf("2. Enter '0' to get result sum from server and exit: \n");
		scanf("%d", &dataBuff);
		int converted = htonl(dataBuff);
		int writeSize = write(clientSocketFD, &converted, sizeof(converted));
		printf("Write: %d, %d\n", writeSize, dataBuff);
		if (writeSize == -1)
		{
			perror("Write");
			exit(EXIT_FAILURE);
		}
		if (dataBuff == 0)
			break;
	}

	memset(readBuff, 0, BUFFER_SIZE);
	int readSize = read(clientSocketFD, readBuff, BUFFER_SIZE);
	printf("Read: %d, %s\n", readSize, readBuff);
	if (readSize == -1)
	{
		perror("Read");
		exit(EXIT_FAILURE);
	}

	int closeStatus = close(clientSocketFD);
	printf("Close: %d\n", closeStatus);
	exit(EXIT_SUCCESS);
}
//gcc -g UnixSocketClient.c -o UnixSocketClient
