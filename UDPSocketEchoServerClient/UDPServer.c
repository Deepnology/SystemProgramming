#include <stdio.h>
#include <ctype.h>
#include <stdlib.h> //exit()
#include <string.h>
#include <sys/socket.h> //socket(), bind(), connect(), recv(), send()
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> //close()
#include <errno.h>
#define SERVER_PORT 9005
int main()
{
	int serverSocketFD = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	printf("Socket: %d\n", serverSocketFD);
	if (serverSocketFD == -1)
	{
		printf("Socket");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in servAddr;
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(SERVER_PORT);
	servAddr.sin_addr.s_addr = INADDR_ANY;//means any interface ip address of this machine

	int bindStatus = bind(serverSocketFD, (struct sockaddr*) &servAddr, sizeof(servAddr));
	printf("Bind: %d\n", bindStatus);
	if (bindStatus == -1)
	{
		printf("Bind");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in clntAddr;
	int clntAddrLen = sizeof(clntAddr); //this must be specified for recvfrom !!

	for(;;)
	{
		//now waiting for client send echo msg
		char recvBuf[256];
		memset(recvBuf, 0, sizeof(recvBuf));
		int recvSize = recvfrom(serverSocketFD, recvBuf, sizeof(recvBuf), 0,
				(struct sockaddr*)&clntAddr, &clntAddrLen);
		printf("Recv: %d, %s, %u\n", recvSize, inet_ntoa(clntAddr.sin_addr), ntohs(clntAddr.sin_port));
		if (recvSize == -1)
		{
			perror("Recv");
			exit(EXIT_FAILURE);
		}
		printf("RecvMsg: %.*s\n", recvSize, recvBuf);//print recvBuf with len=recvSize
		if (strncmp(recvBuf, "exitserver", strlen("exitserver")) == 0) break;

		//now send client's echo msg back to client
		int sentSize = sendto(serverSocketFD, recvBuf, recvSize, 0,
				(struct sockaddr*)&clntAddr, sizeof(struct sockaddr));
		printf("Send: %d (Recv: %d), %s, %u\n", sentSize, recvSize, inet_ntoa(clntAddr.sin_addr), ntohs(clntAddr.sin_port));
		if (sentSize != recvSize)
		{
			printf("%s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	int closeServStatus = close(serverSocketFD);
	printf("Close: %d\n", closeServStatus);
	if (closeServStatus == -1)
	{
		perror("Close");
		exit(EXIT_FAILURE);
	}
	return 0;
}
