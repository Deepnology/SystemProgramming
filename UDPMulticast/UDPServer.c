#include <stdio.h>
#include <ctype.h>
#include <stdlib.h> //exit()
#include <string.h>
#include <sys/socket.h> //socket(), bind(), connect(), recv(), send()
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> //close()
#include <errno.h>
#include <netdb.h>
#include <memory.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#define DEST_IP_ADDRESS "127.0.0.1"
struct RecvrThreadFuncArg
{
	int serverSocketFD;
};
static void * RecvrThreadFunc(void * args)
{
	int setCancelState = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	if (setCancelState)
	{
		printf("pthread_setcancelstate failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	struct RecvrThreadFuncArg * arg = (struct RecvrThreadFuncArg*) args;
	int serverSocketFD = arg->serverSocketFD;
	for (;;)
	{
		char recvBuf[256];
		memset(recvBuf, 0, sizeof(recvBuf));
		struct sockaddr_in clntAddr;
		int clntAddrLen = sizeof(clntAddr);
		int recvSize = recvfrom(serverSocketFD, recvBuf, sizeof(recvBuf), 0,
				(struct sockaddr*)&clntAddr, &clntAddrLen);
		printf("Recv: %d, %s, %u: %.*s\n", recvSize, inet_ntoa(clntAddr.sin_addr), ntohs(clntAddr.sin_port),
				recvSize, recvBuf);
		if (recvSize == -1)
		{
			perror("Recv");
			exit(EXIT_FAILURE);
		}
	}
}
int main()
{
	int SERVER_PORT;
	printf("Enter Server Port (start from 9000): ");
	scanf("%d", &SERVER_PORT);
	int TOTAL_SERVER;
	printf("Enter Number of Server: ");
	scanf("%d", &TOTAL_SERVER);
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

	pthread_t recvrThreadId;
	pthread_attr_t recvrThreadAttr;
	pthread_attr_init(&recvrThreadAttr);
	struct RecvrThreadFuncArg * recvrThreadArg = (struct RecvrThreadFuncArg*)calloc(1, sizeof(struct RecvrThreadFuncArg));
	memcpy(&recvrThreadArg->serverSocketFD, &serverSocketFD, sizeof(int));
	int pthreadCreateErr = pthread_create(&recvrThreadId, &recvrThreadAttr, RecvrThreadFunc, (void*)recvrThreadArg);
	if (pthreadCreateErr)
	{
		printf("pthread_create fail: %s\n", strerror(errno));
		return pthreadCreateErr;
	}


	for (;;)
	{
		char buf[256];
		memset(buf, 0, sizeof(buf));
		printf("Enter text to multicast: ");
		scanf("%s", buf);
		if (strncmp(buf, "exit", strlen("exit")) == 0) break;
		for (int i = 0; i < TOTAL_SERVER; ++i)
		{
			if (9000+i == SERVER_PORT) continue;

			struct sockaddr_in destAddr;
			memset(&destAddr, 0, sizeof(destAddr));
			destAddr.sin_family = AF_INET;
			destAddr.sin_port = htons(9000+i);
			destAddr.sin_addr.s_addr = inet_addr(DEST_IP_ADDRESS);
			int sentSize = sendto(serverSocketFD, buf, sizeof(buf), 0,
					(struct sockaddr*)&destAddr, sizeof(struct sockaddr));
			if (sentSize == -1)
			{
				printf("%s\n", strerror(errno));
				exit(EXIT_FAILURE);
			}
		}
	}


	int closeStatus = close(serverSocketFD);
	printf("Close: %d\n", closeStatus);
	if (closeStatus == -1)
	{
		perror("Close");
		exit(EXIT_FAILURE);
	}

	int pthreadCancelErr = pthread_cancel(recvrThreadId);
	printf("Pthread Cancel: %d\n", pthreadCancelErr);
	if (pthreadCancelErr)
	{
		printf("pthread_cancel failed: %s\n", strerror(errno));
		return pthreadCancelErr;
	}
	void * joinStatus;
	int pthreadJoinErr = pthread_join(recvrThreadId, &joinStatus);
	printf("Pthread Join: %d\n", pthreadJoinErr);
	if (pthreadJoinErr)
	{
		printf("pthread_join failed: %s\n", strerror(errno));
		return pthreadJoinErr;
	}
	free(recvrThreadArg);
	return 0;
}
/*
gcc UDPServer.c  -o UDPServer -lpthread -lrt
valgrind --leak-check=full ./UDPServer
 */
