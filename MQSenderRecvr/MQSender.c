#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <errno.h>

#define MAX_MSG_SIZE 256
#define MSG_BUFFER_SIZE (MAX_MSG_SIZE + 10)

int main(int argc, char * argv[])
{
	char buff[MSG_BUFFER_SIZE];

	if (argc <= 1)
	{
		printf("Provide a reciepient msgQ name: format </msgq-name>\n");
		exit(EXIT_FAILURE);
	}
	char * mqName = argv[1];

	int mqFD = mq_open(mqName, O_WRONLY | O_CREAT, NULL, NULL);
	printf("Open: %d\n", mqFD);
	if (mqFD == -1)
	{
		perror("Open");
		exit(EXIT_FAILURE);
	}

	for (;;)
	{
		memset(buff, 0, MSG_BUFFER_SIZE);
		printf("Enter message to send to receiver '%s', or enter 'exit' to exit: \n", mqName);
		scanf("%s", buff);
		int sendSize = mq_send(mqFD, buff, strlen(buff)+1, 0);
		printf("Send: %d, %s\n", sendSize, buff);
		if (sendSize == -1)
		{
			perror("Send");
			exit(EXIT_FAILURE);
		}
		if (strncmp(buff, "exit", 4) == 0)
			break;
	}

	int closeStatus = mq_close(mqFD);
	printf("Close: %d\n", closeStatus);
	exit(EXIT_SUCCESS);
}

//gcc -g -c MQsender.c -o MQsender.o
//gcc -g MQsender.o -o MQsender -lrt
