#include <stdio.h>
#include <stdlib.h> //exit()
#include <string.h>
#include <sys/socket.h> //socket(), bind(), connect(), recv(), send()
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h> //close()
#include <errno.h>
int main()
{
	FILE * htmlData = fopen("index.html", "r");
	char responseData[1024];
	fgets(responseData, 1024, htmlData);
	char httpHeader[2048] = "HTTP/1.1 200 OK\r\n\n";
	strcat(httpHeader, responseData);

	int serverSocketFD = socket(AF_INET, SOCK_STREAM, 0);
	printf("Socket: %d\n", serverSocketFD);
	if (serverSocketFD == -1)
	{
		perror("Socket");
		exit(EXIT_FAILURE);
	}
	struct sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(8001);
	servAddr.sin_addr.s_addr = INADDR_ANY;
	
	int bindStatus = bind(serverSocketFD, (struct sockaddr*) &servAddr, sizeof(servAddr));
	printf("Bind: %d\n", bindStatus);
	if (bindStatus == -1)
	{
		perror("Bind");
		exit(EXIT_FAILURE);
	}
	int listenStatus = listen(serverSocketFD, 5);
	printf("Listen: %d\n", listenStatus);
	if (listenStatus == -1)
	{
		perror("Listen");
		exit(EXIT_FAILURE);
	}
	for (;;)
	{
		printf("Accept ....\n");
		int clientSocketFD = accept(serverSocketFD, NULL, NULL);
		printf("Accept: %d\n", clientSocketFD);
		if (clientSocketFD == -1)
		{
			perror("Accept");
			exit(EXIT_FAILURE);
		}
		int sentSize = send(clientSocketFD, httpHeader, sizeof(httpHeader), 0);
		printf("Send: %d\n", sentSize);
		if (sentSize == -1)
		{
			perror("Send");
			exit(EXIT_FAILURE);
		}
		int closeStatus = close(clientSocketFD);
		printf("Close: %d\n", closeStatus);
		if (closeStatus == -1)
		{
			perror("Close");
			exit(EXIT_FAILURE);
		}
	}

	int closeStatus = close(serverSocketFD);
	printf("Close: %d\n", closeStatus);
	if (closeStatus == -1)
	{
		perror("Close");
		exit(EXIT_FAILURE);
	}
	return 0;
}
//open browser and browse to
//127.0.0.1:8001
