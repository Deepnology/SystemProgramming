#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <memory.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#define SERVER_PORT 2000
char buff[1024];//for recvfrom

typedef struct student_
{
	char name[32];
	unsigned int roll_no;
	char hobby[32];
	char dept[32];
} student_t;
student_t student[5] = 
{
	{"David Fang", 10305042, "Programming", "CSE"},
	{"Robert Levine", 10305048, "Programming", "CSE"},
	{"Aparajith Sairam", 10305041, "Cricket", "CSE"},
	{"Jeff Kelsey", 10305032, "Udemy Teaching", "ME"},
	{"Spyros Dimis", 10305030, "Art", "MA"},
};

void TrimSpacesBothSides(char * s)
{
	if (s == NULL) return;
	char * ptr = s;
	int len = strlen(s);
	if (len == 0) return;
	if (!isspace(ptr[0]) && !isspace(ptr[len-1])) return;
	printf("TrimSpacesBothSides for '%s': ", s);
	//now at least one side has space
	while (len-1 >= 0 && isspace(ptr[len-1]))
		ptr[--len] = 0;
	printf("len=%d ", len);
	while (*ptr && isspace(*ptr))
		++ptr, --len;
	printf("len=%d ", len);
	memmove(s, ptr, len+1);//len+1 is to add a null terminator
	//void * memmove(void* dest, void* src, size_t count)
	//copy count of chars from the object pointed by src to the object pointed by dest
	//return dest
	printf("'%s'\n", s);
}
static char * processGET(char * URL, unsigned int * resultLen)
{
	printf("%s(%u) called with URL = %s\n", __FUNCTION__, __LINE__, URL);

	//URL: /College/IIT/?dept=CSE&rollno=10305024/
	char delimiter[2] = {'?', '\0'};
	TrimSpacesBothSides(URL);
	char * token[5] = {0};
	token[0] = strtok(URL, delimiter);// /College/IIT/
	token[1] = strtok(NULL, delimiter);// dept=CSE&rollno=10305024/
	delimiter[0] = '&';
	token[2] = strtok(token[1], delimiter);// dept=CSE
	token[3] = strtok(NULL, delimiter);// rollno=10305024
	delimiter[0] = '=';
	char * rollno = strtok(token[3], delimiter);
	char * rollnoVal = strtok(NULL, delimiter);
	unsigned int val = atoi(rollnoVal);

	int i;
	for (i = 0; i < 5; ++i)
	{
		if (student[i].roll_no == val)
			break;
	}
	if (i == 5) return NULL;

	char * result = calloc(1, 1024);
	strcpy(result,
			"<html>"
			"<head>"
				"<title>HTML Response</title>"
				"<style>"
				"table, th, td {"
					"border: 1px solid black;}"
				"</style>"
			"</head>"
			"<body>"
			"<table>"
			"<tr>"
			"<td>");
	strcat(result, student[i].name);
	strcat(result,
		"</table>"
		"</body>"
		"</html>");
	
	char * header = calloc(1, 248 + strlen(result));
	strcpy(header, "HTTP/1.1 200 OK\n");
	strcat(header, "Server: My Personal HTTP Server\n");
	strcat(header, "Content-Length: ");
	strcat(header, "Connection: close\n");
	strcat(header, "2048");
	strcat(header, "\n");
	strcat(header, "Content-Type: text/html; charset=UTF-8\n");
	strcat(header, "\n");

	strcat(header, result);
	*resultLen = strlen(header);
	free(result);
	return header;
}
static char * processPOST(char * URL, unsigned int * resultLen)
{
	return NULL;
}

int main(int argc, char * argv[])
{

	int masterSocketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	printf("Socket: %d\n", masterSocketFD);
	if (masterSocketFD == -1)
	{
		perror("Socket");
		exit(EXIT_FAILURE);
	}

	int opt = 1;
	int setSockOptStatus = setsockopt(masterSocketFD, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
	printf("SetSockOpt to allow multiple connections: %d\n", setSockOptStatus);
	if (setSockOptStatus == -1)
	{
		perror("SetSockOpt");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;//only ipv4 network packets
	servAddr.sin_port = htons(SERVER_PORT);
	servAddr.sin_addr.s_addr = INADDR_ANY;
	int bindStatus = bind(masterSocketFD, (struct sockaddr*)&servAddr, sizeof(struct sockaddr));
	printf("Bind: %d\n", bindStatus);
	if (bindStatus == -1)
	{
		perror("Bind");
		exit(EXIT_FAILURE);
	}

	int listenStatus = listen(masterSocketFD, 5);
	if (listenStatus == -1)
	{
		perror("Listen");
		exit(EXIT_FAILURE);
	}

	fd_set fdSet;
	for (;;)
	{
		FD_ZERO(&fdSet);
		FD_SET(masterSocketFD, &fdSet);
		printf("Select ....\n");
		int selectedFD = select(masterSocketFD+1, &fdSet, NULL, NULL, NULL);
		printf("Select: %d\n", selectedFD);
		if (selectedFD == -1)
		{
			perror("Select");
			exit(EXIT_FAILURE);
		}

		if (FD_ISSET(masterSocketFD, &fdSet) != 0)//perform accept
		{
			printf("Accept ....\n");
			struct sockaddr_in clientAddr;
			unsigned int clientAddrLen = sizeof(struct sockaddr);
			int clientSocketFD = accept(masterSocketFD, (struct sockaddr*)&clientAddr, &clientAddrLen);
			printf("Accept: %d, %s, %u\n", clientSocketFD, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
			if (clientSocketFD == -1)
			{
				perror("Accept");
				exit(EXIT_FAILURE);
			}

			for (;;)
			{
				memset(buff, 0, sizeof(buff));
				int recvSize = recvfrom(clientSocketFD, (char*)&buff, sizeof(buff), 0, (struct sockaddr*)&clientAddr, &clientAddrLen);
				printf("Recvfrom: %d, %s, %u\n", recvSize, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
				printf("%s\n", buff);
				if (recvSize == -1)
				{
					perror("Recvfrom");
					exit(EXIT_FAILURE);
				}
				if (recvSize == 0)
				{
					int closeStatus = close(clientSocketFD);
					printf("Close: %d\n", closeStatus);
					if (closeStatus == -1)
					{
						perror("Close");
						exit(EXIT_FAILURE);
					}
					break;
				}

				//parse the received msg to GET or POST
				char * requestLine = NULL;
				char delimiter[2] = "\n";
				char * method = NULL;//the HTTP method in the requestLine
				char * URL = NULL;//the URL in the requestLine
				requestLine = strtok(buff, delimiter);
				delimiter[0] = ' ';
				method = strtok(requestLine, delimiter);
				URL = strtok(NULL, delimiter);
				printf("Method: %s\n", method);
				printf("URL: %s\n", URL);

				//process GET or POST
				char * response = NULL;//server response to client
				unsigned int responseLen = 0;
				if (strncmp(method, "GET", strlen("GET")) == 0)
					response = processGET(URL, &responseLen);
				else if (strncmp(method, "POST", strlen("POST")) == 0)
					response = processPOST(URL, &responseLen);
				else
				{
					printf("Unsupported URL method request: %s\n", method);
					int closeStatus = close(clientSocketFD);
					printf("Close: %d\n", closeStatus);
					if (closeStatus == -1)
					{
						perror("Close");
						exit(EXIT_FAILURE);
					}
					break;
				}

				//server send response to client
				if (response)
				{
					int sentSize = sendto(clientSocketFD, response, responseLen, 0, (struct sockaddr*)&clientAddr, clientAddrLen);
					printf("Send: %d\n", sentSize);
					if (sentSize == -1)
					{
						perror("Send");
						exit(EXIT_FAILURE);
					}
					free(response);
				}

			}//end of for(;;)
		}
		else
		{

		}
	}//end of for(;;)

	return 0;
}
//open browser and browse to
//127.0.0.1:2000/College/IIT/?dept=CSE&rollno=10305042
