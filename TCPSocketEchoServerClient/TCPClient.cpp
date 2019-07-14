#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <memory.h>
#include <string>
#include <iostream>
#define DEST_PORT 9002
#define DEST_IP_ADDRESS "127.0.0.1"
int main()
{
	int clientSocketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	printf("Socket: %d\n", clientSocketFD);
	if (clientSocketFD == -1)
	{
		perror("Socket");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in destAddr;
	destAddr.sin_family = AF_INET;
	destAddr.sin_port = htons(DEST_PORT);
	struct hostent *host = (struct hostent*) gethostbyname(DEST_IP_ADDRESS);
	destAddr.sin_addr = *((struct in_addr*)host->h_addr);

	int connectStatus = connect(clientSocketFD, (struct sockaddr*)&destAddr, sizeof(struct sockaddr));
	printf("Connect: %d\n", connectStatus);
	if (connectStatus == -1)
	{
		perror("Connect");
		exit(EXIT_FAILURE);
	}

	//receive greetings from server
	char recvBuf[256] = {'\0'};
	int recvSize;
       	recvSize = recv(clientSocketFD, &recvBuf, sizeof(recvBuf), 0);
	printf("Recv: %d\n", recvSize);
	if (recvSize == -1)
	{
		printf("Recv");
		exit(EXIT_FAILURE);
	}
	printf("RecvMsg: %.*s\n", recvSize, recvBuf);

	for(;;)
	{
		std::string buf;
		printf("Enter text to echo, or 'exit' to exit: ");
		getline(std::cin, buf);
		int sentSize = send(clientSocketFD, buf.c_str(), buf.size()+1, 0);//+1: to indicate a null end
		printf("Send: %d\n", sentSize);
		if (sentSize == -1)
		{
			printf("%s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		if (buf == "exit") break;
		recvSize = recv(clientSocketFD, &recvBuf, sizeof(recvBuf), 0);
		if (recvSize == -1)
		{
			perror("Recv");
			exit(EXIT_FAILURE);
		}
		printf("RecvMsg: %.*s\n", recvSize, recvBuf);
	}

	int closeStatus = close(clientSocketFD);
	printf("Close: %d\n", closeStatus);
	if (closeStatus == -1)
	{
		perror("Close");
		exit(EXIT_FAILURE);
	}
	return 0;
}
