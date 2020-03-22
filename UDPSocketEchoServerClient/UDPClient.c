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
#define DEST_PORT 9005
#define DEST_IP_ADDRESS "127.0.0.1" 
int main()
{
	int clientSocketFD = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	printf("Socket: %d\n", clientSocketFD);
	if (clientSocketFD == -1)
	{
		perror("Socket");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in servAddr;
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(DEST_PORT);
	//struct hostent * host = (struct hostent*) gethostbyname(DEST_IP_ADDRESS);
	//memcpy((char*)&servAddr.sin_addr, (char*)host->h_addr, host->h_length);
	//servAddr.sin_addr.s_addr = INADDR_ANY;
	servAddr.sin_addr.s_addr = inet_addr(DEST_IP_ADDRESS);
	printf("Sendto: %d, %s, %u\n", servAddr.sin_family, inet_ntoa(servAddr.sin_addr), ntohs(servAddr.sin_port));
	unsigned int servAddrLen;

	for(;;)
	{
		char buf[256];
		memset(buf, 0, sizeof(buf));
		printf("Enter text to echo, or 'exit' to exit, or 'exitserver' to exit server: ");
		scanf("%s", buf);
		int sentSize = sendto(clientSocketFD, buf, sizeof(buf), 0,
				(struct sockaddr*)&servAddr, sizeof(struct sockaddr));
		printf("Send: %d\n", sentSize);
		if (sentSize == -1)
		{
			printf("%s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		if (strncmp(buf, "exit", strlen("exit")) == 0) break;

		char recvBuf[256];
		memset(recvBuf, 0, sizeof(recvBuf));
		int recvSize = recvfrom(clientSocketFD, recvBuf, sizeof(recvBuf), 0,
				(struct sockaddr*)&servAddr, &servAddrLen);
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
