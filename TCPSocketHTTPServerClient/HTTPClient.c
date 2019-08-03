#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
int main(int argc, char * argv[])
{
	if (argc < 2)
	{
		printf("Please provide an ip address ...\n");
		exit(EXIT_FAILURE);
	}
	char * addr = argv[1];

	int clientSocketFD = socket(AF_INET, SOCK_STREAM, 0);
	printf("Socket: %d\n", clientSocketFD);
	if (clientSocketFD == -1)
	{
		perror("Socket");
		exit(EXIT_FAILURE);
	}
	struct sockaddr_in remoteAddr;
	remoteAddr.sin_family = AF_INET;
	remoteAddr.sin_port = htons(80);//port 80 is the most website on
	inet_aton(addr, &remoteAddr.sin_addr);//convert addr from string to the target type and store
	
	int connectStatus = connect(clientSocketFD, (struct sockaddr*) &remoteAddr, sizeof(remoteAddr));
	printf("Connect: %d\n", connectStatus);
	if (connectStatus == -1)
	{
		perror("Connect");
		exit(EXIT_FAILURE);
	}
	char request[] = "GET / HTTP/1.1\r\n\r\n";
	int sentSize = send(clientSocketFD, request, sizeof(request), 0);
	printf("Send: %d\n", sentSize);
	if (sentSize == -1)
	{
		perror("Send");
		exit(EXIT_FAILURE);
	}
	char recvBuf[4096] = {'\0'};
	int recvSize = recv(clientSocketFD, &recvBuf, sizeof(recvBuf), 0);
	printf("Recv: %d\n", recvSize);
	if (recvSize == -1)
	{
		perror("Recv");
		exit(EXIT_FAILURE);
	}
	recvBuf[recvSize] = '\0';
	printf("HTTPclient ReceivedMsg: %s\n", recvBuf);

	int closeStatus = close(clientSocketFD);
	printf("Close: %d\n", closeStatus);
	if (closeStatus == -1)
	{
		perror("Close");
		exit(EXIT_FAILURE);
	}
	return 0;
}
