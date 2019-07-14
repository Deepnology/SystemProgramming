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
#include <set>
#define SERVER_PORT 9002
std::set<int> mFDSet;
int main()
{
	printf("C++ version: %ld\n", __cplusplus);
	int serverSocketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	printf("Socket: %d\n", serverSocketFD);
	if (serverSocketFD == -1)
	{
		printf("Socket");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in servAddr;
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

	int listenStatus = listen(serverSocketFD, 10);//maintain a max number of client connections in the queue (will drop when exceeding the max num)
	printf("Listen: %d\n", listenStatus);
	if (listenStatus == -1)
	{
		printf("Listen");
		exit(EXIT_FAILURE);
	}

	mFDSet.insert(serverSocketFD);
	fd_set readFDSet;

	for(;;)
	{
		FD_ZERO(&readFDSet);
		for (auto & fd : mFDSet)
			FD_SET(fd, &readFDSet);

		printf("Select ....\n");
		int selectedFDCount = select(*mFDSet.rbegin()+1, &readFDSet, NULL, NULL, NULL);
		printf("Select: %d\n", selectedFDCount);
		if (selectedFDCount == -1)
		{
			perror("Select");
			exit(EXIT_FAILURE);
		}
		if (FD_ISSET(serverSocketFD, &readFDSet))//serverSocketFD was selected: perform accept
		{
			struct sockaddr_in clntAddr;
			unsigned int clntAddrLen = sizeof(clntAddr);
			printf("Accept ....\n");
			int clientSocketFD = accept(serverSocketFD, (struct sockaddr*) &clntAddr, &clntAddrLen);
			printf("Accept: %d, %s, %u\n", clientSocketFD, inet_ntoa(clntAddr.sin_addr), ntohs(clntAddr.sin_port));
			if (clientSocketFD == -1)
			{
				perror("Accept");
				exit(EXIT_FAILURE);
			}

			char servMsg[] = "You have reached server!";
			//int sentSize = send(clientSocketFD, servMsg, sizeof(servMsg), 0);//equivalent to sendto
			int sentSize = sendto(clientSocketFD, servMsg, sizeof(servMsg), 0,
					(struct sockaddr*)&clntAddr, sizeof(struct sockaddr));
			printf("Send: %d, %s, %u\n", sentSize, inet_ntoa(clntAddr.sin_addr), ntohs(clntAddr.sin_port));
			if (sentSize == -1)
			{
				perror("Send");
				exit(EXIT_FAILURE);
			}
			mFDSet.insert(clientSocketFD);
		}
		else//serverSocketFD is not set: mulitple FDs from readFDSet/writeFDSet/exceptFDSet might be set
		{
			for (auto itr = mFDSet.begin(); itr != mFDSet.end(); )
			{
				int clientSocketFD = *itr;
				if (!FD_ISSET(clientSocketFD, &readFDSet))
			       	{
					++itr;
					continue;
				}

				//now waiting for client send echo msg
				char recvBuf[256];
				memset(recvBuf, 0, sizeof(recvBuf));
				struct sockaddr_in clntAddr;
				unsigned int clntAddrLen = sizeof(clntAddr);
				printf("Recv ....\n");
				//int recvSize = recv(clientSocketFD, (char*)&recvBuf, sizeof(recvBuf), 0);//equivalent to recvfrom
				int recvSize = recvfrom(clientSocketFD, (char*)&recvBuf, sizeof(recvBuf), 0,
						(struct sockaddr*)&clntAddr, &clntAddrLen);
				printf("Recv: %d, %s, %u\n", recvSize, inet_ntoa(clntAddr.sin_addr), ntohs(clntAddr.sin_port));
				if (recvSize == -1)
				{
					perror("Recv");
					exit(EXIT_FAILURE);
				}
				printf("RecvMsg: %.*s\n", recvSize, recvBuf);//print recvBuf with len=recvSize

				if (strncmp(recvBuf, "exit", strlen("exit")) != 0)
				{
					//now send client's echo msg back to client
					//sentSize = send(clientSocketFD, recvBuf, recvSize, 0);//equivalent to sendto
					int sentSize = sendto(clientSocketFD, recvBuf, recvSize, 0,
						(struct sockaddr*)&clntAddr, sizeof(struct sockaddr));
					printf("Send: %d (Recv: %d), %s, %u\n", sentSize, recvSize, inet_ntoa(clntAddr.sin_addr), ntohs(clntAddr.sin_port));
					if (sentSize != recvSize)
					{
						printf("%s\n", strerror(errno));
						exit(EXIT_FAILURE);
					}
					++itr;
				}
				else
				{
					int closeClntStatus = close(clientSocketFD);
					printf("Close: %d\n", closeClntStatus);
					if (closeClntStatus == -1)
					{
						perror("Close");
						exit(EXIT_FAILURE);
					}
					itr = mFDSet.erase(itr);
				}
			}//end for(fd : mFDSet)

		}//end if(FD_ISSET())
		
	}

	int closeServStatus = close(serverSocketFD);
	printf("Close: %d\n", closeServStatus);
	if (closeServStatus == -1)
	{
		perror("Close");
		exit(EXIT_FAILURE);
	}
	mFDSet.erase(serverSocketFD);
	return 0;
}
