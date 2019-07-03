#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#define SOCKET_NAME "/tmp/DemoUnixSocket"
#define BUFFER_SIZE 128

int main(int argc, char * argv[])
{
	char buff[BUFFER_SIZE];
	int dataBuff;
	int result;

	unlink(SOCKET_NAME);
	int servSocketFD = socket(AF_UNIX, SOCK_STREAM, 0);
	printf("Socket: %d\n", servSocketFD);
	if (servSocketFD == -1)
	{
		perror("Socket");//equivalent to printf("Socket: %s\n", strerror(errno))
		exit(EXIT_FAILURE);
	}

	struct sockaddr_un servAddr;
	memset(&servAddr, 0, sizeof(struct sockaddr_un));
	servAddr.sun_family = AF_UNIX;
	strncpy(servAddr.sun_path, SOCKET_NAME, sizeof(servAddr.sun_path)-1);
	int bindStatus = bind(servSocketFD, (struct sockaddr*) &servAddr, sizeof(struct sockaddr_un));
	printf("Bind: %d\n", bindStatus);
	if (bindStatus == -1)
	{
		perror("Bind");
		exit(EXIT_FAILURE);
	}

	int listenStatus = listen(servSocketFD, 20);//20=max clients that can be queued. client sockets num greater than 20 will be dropped.
	printf("Listen: %d\n", listenStatus);
	if (listenStatus == -1)
	{
		perror("Listen");
		exit(EXIT_FAILURE);
	}

	
	for (;;)
	{
		printf("Accept ....\n");
		int clientSocketFD = accept(servSocketFD, NULL, NULL);
		printf("Accept: %d\n", clientSocketFD);
		if (clientSocketFD == -1)
		{
			perror("Accept");
			exit(EXIT_FAILURE);
		}
		
		result = 0;
		for(;;)
		{
			printf("Read from %d .... \n", clientSocketFD);
			int readSize = read(clientSocketFD, &dataBuff, sizeof(dataBuff));
			dataBuff = ntohl(dataBuff);
			printf("Read: %d, %d\n", readSize, dataBuff);
			if (readSize = -1)
			{
				printf("Read: %d, %s\n", readSize, strerror(errno));
				perror("Read");
				//exit(EXIT_FAILURE);
			}
			if (dataBuff == 0) 
				break;
			result += dataBuff;
		}

		memset(buff, 0, BUFFER_SIZE);
		sprintf(buff, "Result = %d", result);
		int writeSize = write(clientSocketFD, buff, BUFFER_SIZE);
		printf("Write: %d, %s\n", writeSize, buff);
		if (writeSize == -1)
		{
			perror("Write");
			exit(EXIT_FAILURE);
		}
						
		int closeStatus = close(clientSocketFD);
		printf("Close: %d\n", closeStatus);

	}//end for(;;)

	int closeStatus = close(servSocketFD);
	printf("Close: %d\n", closeStatus);
	unlink(SOCKET_NAME);
	exit(EXIT_SUCCESS);

}
//gcc -g UnixSocketServer.c -o UnixSocketServer
