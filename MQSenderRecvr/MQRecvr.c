#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <errno.h>

#define MAX_MSG_COUNT 10
#define MAX_MSG_SIZE 256
#define MSG_BUFFER_SIZE (MAX_MSG_SIZE + 10)
#define MQ_PERMISSIONS 0660

int main(int argc, char * argv[])
{
	char buff[MSG_BUFFER_SIZE];

	if (argc <= 1)
	{
		printf("Provide a reciepient msgQ name: format </msgq-name>\n");
		exit(EXIT_FAILURE);
	}
	char * mqName = argv[1];

	struct mq_attr mqAttr;
	mqAttr.mq_flags = 0;
	mqAttr.mq_maxmsg = MAX_MSG_COUNT;
	mqAttr.mq_msgsize = MAX_MSG_SIZE;
	mqAttr.mq_curmsgs = 0;

	int mqFD = mq_open(mqName, O_RDONLY | O_CREAT, MQ_PERMISSIONS, &mqAttr);
	if (mqFD == -1)
	{
		perror("Open");
		exit(EXIT_FAILURE);
	}

	fd_set fdSet;
	for (;;)
	{
		FD_ZERO(&fdSet);
		FD_SET(mqFD, &fdSet);

		printf("Select ....\n");
		int selected = select(mqFD+1, &fdSet, NULL, NULL, NULL);
		printf("Select: %d\n", selected);
		if (FD_ISSET(mqFD, &fdSet))
		{
			memset(buff, 0, MSG_BUFFER_SIZE);
			int recvSize = mq_receive(mqFD, buff, MSG_BUFFER_SIZE, NULL);
			printf("Receive: %d, %s\n", recvSize, buff);
			if (recvSize == -1)
			{
				perror("Receive");
				exit(EXIT_FAILURE);
			}

			if (strncmp(buff, "exit", 4) == 0)
				break;
		}
	}

	int closeStatus = mq_close(mqFD);
	printf("Close: %d\n", closeStatus);
	int unlinkStatus = mq_unlink(mqName);
	printf("Unlink: %d\n", unlinkStatus);
	exit(EXIT_SUCCESS);
}

//gcc -g -c MQrecvr.c -o MQrecvr.o
//gcc -g MQrecvr.o -o MQrecvr -lrt
