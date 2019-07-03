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
#define MAX_FD_SIZE 32

int monitoredFDArr[MAX_FD_SIZE];//global array holding both server and client socket id FDs
int clientResArr[MAX_FD_SIZE] = {0};//global array holding the computation result for each client

static void initMonitoredFDArr(int monitoredFDArr[])
{
	for (int i = 0; i < MAX_FD_SIZE; ++i)
		monitoredFDArr[i] = -1;
}
static void addMonitoredFDArr(int monitoredFDArr[], int fd)
{
	for (int i = 0; i < MAX_FD_SIZE; ++i)
		if (monitoredFDArr[i] == -1)
		{
			monitoredFDArr[i] = fd;//update the first "-1" from begin to be fd
			break;
		}
}
static void removeMonitoredFDArr(int monitoredFDArr[], int fd)
{
	for (int i = 0; i < MAX_FD_SIZE; ++i)
		if (monitoredFDArr[i] == fd)
		{
			monitoredFDArr[i] = -1;
			break;
		}
}
static void refreshFDSet(fd_set * fdSet, int monitoredFDArr[])//clone all FDs in monitoredFDArr to fd_set
{
	FD_ZERO(fdSet);
	for (int i = 0; i < MAX_FD_SIZE; ++i)
		if (monitoredFDArr[i] != -1)
			FD_SET(monitoredFDArr[i], fdSet);
}
static int getMaxMonitoredFDArr(int monitoredFDArr[])
{
	int res = -1;
	for (int i = 0; i < MAX_FD_SIZE; ++i)
		if (monitoredFDArr[i] > res)
			res = monitoredFDArr[i];
	return res;
}

int main(int argc, char * argv[])
{
	char buff[BUFFER_SIZE];
	int dataBuff;

	initMonitoredFDArr(monitoredFDArr);
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

	addMonitoredFDArr(monitoredFDArr, servSocketFD);
	fd_set fdSet;

	for (;;)
	{
		refreshFDSet(&fdSet, monitoredFDArr);
		printf("Select ....\n");
		int selectedFD = select(getMaxMonitoredFDArr(monitoredFDArr)+1, &fdSet, NULL, NULL, NULL);
		printf("Select: %d\n", selectedFD);
		if (selectedFD == -1)
		{
			perror("Select");
			exit(EXIT_FAILURE);
		}
		if (FD_ISSET(servSocketFD, &fdSet) != 0) //perform accept for servSocketFD
		{
			printf("Accept ....\n");
			int clientSocketFD = accept(servSocketFD, NULL, NULL);
			printf("Accept: %d\n", clientSocketFD);
			if (clientSocketFD == -1)
			{
				perror("Accept");
				exit(EXIT_FAILURE);
			}
			addMonitoredFDArr(monitoredFDArr, clientSocketFD);
		}
		else //perform read and write for clientSocketFD
		{
			for (int i = 0; i < MAX_FD_SIZE; ++i)
			{
				if (monitoredFDArr[i] != -1 && monitoredFDArr[i] != servSocketFD &&
						FD_ISSET(monitoredFDArr[i], &fdSet))//select: block until one or more of the FD become ready
				{
					int clientSocketFD = monitoredFDArr[i];
					memset(buff, 0, BUFFER_SIZE);
					printf("Read from %d .... \n", clientSocketFD);
					int readSize = read(clientSocketFD, buff, BUFFER_SIZE);
					printf("Read: %d, %s\n", readSize, buff);
					if (readSize = -1)
					{
						printf("Read: %d, %s\n", readSize, strerror(errno));
						perror("Read");
						//exit(EXIT_FAILURE);
					}
					memcpy(&dataBuff, buff, sizeof(int));
					dataBuff = ntohl(dataBuff);
					if (dataBuff == 0)
					{
						memset(buff, 0, BUFFER_SIZE);
						sprintf(buff, "Result = %d", clientResArr[i]);
						int writeSize = write(clientSocketFD, buff, BUFFER_SIZE);
						printf("Write: %d, %s\n", writeSize, buff);
						if (writeSize == -1)
						{
							perror("Write");
							exit(EXIT_FAILURE);
						}
						int closeStatus = close(clientSocketFD);
						printf("Close: %d\n", closeStatus);
						clientResArr[i] = 0;
						removeMonitoredFDArr(monitoredFDArr, clientSocketFD);
						continue;
					}
					clientResArr[i] += dataBuff;
				}
			}
		}
	}//end of for(;;)

	int closeStatus = close(servSocketFD);
	printf("Close: %d\n", closeStatus);
	removeMonitoredFDArr(monitoredFDArr, servSocketFD);
	unlink(SOCKET_NAME);
	exit(EXIT_SUCCESS);

}
//gcc -g UnixSocketServerMP.c -o UnixSocketServerMP
