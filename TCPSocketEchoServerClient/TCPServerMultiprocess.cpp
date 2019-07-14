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
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <signal.h>
#define SERVER_PORT 9002

static void SIGINTHandler(int sig)
{
	printf("Main_%d_%d: SIGINT received or Ctrl+C pressed: %d\n", getpid(), getppid(), sig);
	exit(EXIT_SUCCESS);	
}

struct ClientInfo
{
	int SocketFD;
	struct sockaddr_in Addr;
};
static void * ClientHandler(void * arg)
{
	struct ClientInfo * clientInfo = (struct ClientInfo*)arg;
	int clientSocketFD = clientInfo->SocketFD;
	struct sockaddr_in clientAddr = clientInfo->Addr;
	unsigned int addrLen = 0;
	char buff[256];//thread local buff
	for (;;)
	{
		printf("Child_%d_%d Recv ....\n", getpid(), getppid());
		memset(buff, 0, sizeof(buff));
		int recvSize = recvfrom(clientSocketFD, (char*)buff, sizeof(buff), 0,
				(struct sockaddr*)&clientAddr, &addrLen);

		printf("Child_%d_%d Recv: %s\n", getpid(), getppid(), buff);
		if (recvSize == -1)
		{
			perror("Child_Recv");
			exit(EXIT_FAILURE);
		}
		
		if (strncmp(buff, "exit", strlen("exit")) == 0) break;
		int sentSize =sendto(clientSocketFD, buff, recvSize, 0,
				(struct sockaddr*)&clientAddr, sizeof(struct sockaddr));
		printf("Child_%d_%d Send: %d\n", getpid(), getppid(), sentSize);
		if (sentSize != recvSize)
		{
			perror("Child_Send");
			exit(EXIT_FAILURE);
		}
	}
	int closeStatus = close(clientSocketFD);
	printf("Child_%d_%d Close: %d\n", getpid(), getppid(), closeStatus);
	if (closeStatus == -1)
	{
		perror("Child_Close");
		exit(EXIT_FAILURE);
	}
}


int main()
{
	//signal(SIGINT, SIGINTHandler);//ctrl+c

	int serverSocketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	printf("Main_%d_%d Socket: %d\n", getpid(), getppid(), serverSocketFD);
	if (serverSocketFD == -1)
	{
		printf("Main_Socket");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(SERVER_PORT);
	servAddr.sin_addr.s_addr = INADDR_ANY;//means any interface ip address of this machine

	int bindStatus = bind(serverSocketFD, (struct sockaddr*) &servAddr, sizeof(servAddr));
	printf("Main_%d_%d Bind: %d\n", getpid(), getppid(), bindStatus);
	if (bindStatus == -1)
	{
		printf("Main_Bind");
		exit(EXIT_FAILURE);
	}

	int listenStatus = listen(serverSocketFD, 5);//maintain a max number of client connections in the queue (will drop when exceeding the max num)
	printf("Main_%d_%d Listen: %d\n", getpid(), getppid(), listenStatus);
	if (listenStatus == -1)
	{
		printf("Main_Listen");
		exit(EXIT_FAILURE);
	}

	fd_set readFDSet;

	for(;;)
	{
		FD_ZERO(&readFDSet);
		FD_SET(serverSocketFD, &readFDSet);
		printf("Main_%d_%d Select ....\n", getpid(), getppid());
		int selectedFD = select(serverSocketFD+1, &readFDSet, NULL, NULL, NULL);
		printf("Main_%d_%d Select: %d\n", getpid(), getppid(), selectedFD);
		if (selectedFD == -1)
		{
			perror("Main_Select");
			exit(EXIT_FAILURE);
		}
		if (FD_ISSET(serverSocketFD, &readFDSet))//serverSocketFD was selected: perform accept
		{
			struct sockaddr_in clntAddr;
			unsigned int clntAddrLen = 0;
			printf("Main_%d_%d Accept ....\n", getpid(), getppid());
			int clientSocketFD = accept(serverSocketFD, (struct sockaddr*) &clntAddr, &clntAddrLen);
			printf("Main_%d_%d Accept: %d, %s, %u\n", getpid(), getppid(), clientSocketFD, inet_ntoa(clntAddr.sin_addr), ntohs(clntAddr.sin_port));
			if (clientSocketFD == -1)
			{
				perror("Main_Accept");
				exit(EXIT_FAILURE);
			}

			char servMsg[] = "You have reached server!";
			//int sentSize = send(clientSocketFD, servMsg, sizeof(servMsg), 0);//equivalent to sendto
			int sentSize = sendto(clientSocketFD, servMsg, sizeof(servMsg), 0,
					(struct sockaddr*)&clntAddr, sizeof(struct sockaddr));
			printf("Main_%d_%d Send: %d, %s, %u\n", getpid(), getppid(), sentSize, inet_ntoa(clntAddr.sin_addr), ntohs(clntAddr.sin_port));
			if (sentSize == -1)
			{
				perror("Main_Send");
				exit(EXIT_FAILURE);
			}

			struct ClientInfo * clientInfo = (struct ClientInfo*)calloc(1, sizeof(struct ClientInfo));
			clientInfo->SocketFD = clientSocketFD;
			memcpy(&clientInfo->Addr, &clntAddr, sizeof(struct sockaddr_in));
			
			int pid = fork();
			if (pid == 0)//child process
			{
				ClientHandler(clientInfo);
				exit(EXIT_SUCCESS);
			}
		}
		else//serverSocketFD is not set
		{
			printf("Main_%d_%d delegate to Child ...\n", getpid(), getppid());
		}
	}

	int closeServStatus = close(serverSocketFD);
	printf("Main_%d_%d Close: %d\n", getpid(), getppid(), closeServStatus);
	if (closeServStatus == -1)
	{
		perror("Main_Close");
		exit(EXIT_FAILURE);
	}
	return 0;
}
//g++ TCPServerMultiprocess.cpp -o TCPServerMultiprocess
