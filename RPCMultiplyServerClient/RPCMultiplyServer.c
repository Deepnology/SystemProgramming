#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory.h>
#include "RPCCommon.h"
#include "../De_Serialization/Serialize.h"

int multiply(int a, int b) { return a*b; }

void rpc_hdr_server_stub_unmarshal(ser_buff_t * server_recv_ser_buff, rpc_hdr_t * rpc_hdr) //de-serialize rpc_hdr
{
	de_serialize_data((char*)&rpc_hdr->op_id, server_recv_ser_buff, sizeof(unsigned int));
	de_serialize_data((char*)&rpc_hdr->payload_size, server_recv_ser_buff, sizeof(unsigned int));
}
void multiply_server_stub_unmarshal(ser_buff_t * server_recv_ser_buff, int * a, int * b) //de-serialize
{
	de_serialize_data((char*)a, server_recv_ser_buff, sizeof(int));
	de_serialize_data((char*)b, server_recv_ser_buff, sizeof(int));
}
void multiply_server_stub_marshal(int res, ser_buff_t * server_send_ser_buff) //serialize
{
	serialize_data(server_send_ser_buff, (char*)&res, sizeof(int));
}

void rpc_server_process_msg(ser_buff_t * server_recv_ser_buff, ser_buff_t * server_send_ser_buff) //de-serialize, multiply, serialize
{
	rpc_hdr_t rpc_hdr;
	rpc_hdr_server_stub_unmarshal(server_recv_ser_buff, &rpc_hdr); //de-serialize rpc_hdr
	printf("RPC_HDR: %d, %d\n", rpc_hdr.op_id, rpc_hdr.payload_size);

	if (rpc_hdr.op_id == RPC_MULTIPLY_ID)
	{
		int a, b;
		multiply_server_stub_unmarshal(server_recv_ser_buff, &a, &b); //de-serialize
		int res = multiply(a, b);
		printf("%d * %d = %d\n", a, b, res);
		multiply_server_stub_marshal(res, server_send_ser_buff); //serialize
	}
	else
	{ }
}

int main(int argc, char * argv[])
{
	int masterSocketFD = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	printf("Socket: %d\n", masterSocketFD);
	if (masterSocketFD == -1)
	{
		perror("Socket");
		exit(EXIT_FAILURE);
	}

	int opt = 1;
	int setSockOptStatus = setsockopt(masterSocketFD, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
	printf("SetSockOpt: %d\n", setSockOptStatus);
	if (setSockOptStatus == -1)
	{
		perror("SetSockOpt");
		exit(EXIT_FAILURE);
	}

	setSockOptStatus = setsockopt(masterSocketFD, SOL_SOCKET, SO_REUSEPORT, (char*)&opt, sizeof(opt));
	printf("SetSockOpt: %d\n", setSockOptStatus);
	if (setSockOptStatus == -1)
	{
		perror("SetSockOpt");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(SERVER_PORT);
	servAddr.sin_addr.s_addr = INADDR_ANY;
	int bindStatus = bind(masterSocketFD, (struct sockaddr*)&servAddr, sizeof(struct sockaddr));
	printf("Bind: %d\n", bindStatus);
	if (bindStatus == -1)
	{
		perror("Bind");
		exit(EXIT_FAILURE);
	}


	ser_buff_t * server_recv_ser_buff = NULL;
	ser_buff_t * server_send_ser_buff = NULL;
	init_serialize_buffer_of_defined_size(&server_recv_ser_buff, MAX_RECV_SEND_BUFF_SIZE);
	init_serialize_buffer_of_defined_size(&server_send_ser_buff, MAX_RECV_SEND_BUFF_SIZE);

	for (;;)
	{
		reset_serialize_buffer(server_recv_ser_buff);
		struct sockaddr_in clientAddr;
		unsigned int clientAddrLen = sizeof(struct sockaddr); //this must be specified !
		int recvSize = recvfrom(masterSocketFD, server_recv_ser_buff->b, get_serialize_buffer_size(server_recv_ser_buff),
				0, (struct sockaddr*)&clientAddr, &clientAddrLen);
		printf("Recvfrom: %d\n", recvSize);
		if (recvSize == -1)
		{
			perror("Recvfrom");
			exit(EXIT_FAILURE);
		}

		reset_serialize_buffer(server_send_ser_buff);
		rpc_server_process_msg(server_recv_ser_buff, server_send_ser_buff); //de-serialize, multiply, serialize

		int sentSize = sendto(masterSocketFD, server_send_ser_buff->b, get_serialize_buffer_data_size(server_send_ser_buff),
				0, (struct sockaddr*)&clientAddr, sizeof(struct sockaddr));
		printf("Sendto: %d\n", sentSize);
		if (sentSize == -1)
		{
			perror("Sendto");
			exit(EXIT_FAILURE);
		}
	}

	return 0;
}


/*
gcc -g -c ../De_Serialization/Serialize.c -o ../De_Serialization/Serialize.o
gcc -g -c RPCMultiplyClient.c -o RPCMultiplyClient.o
gcc -g RPCMultiplyClient.o ../De_Serialization/Serialize.o -o RPCMultiplyClient
gcc -g -c RPCMultiplyServer.c -o RPCMultiplyServer.o
gcc -g RPCMultiplyServer.o ../De_Serialization/Serialize.o -o RPCMultiplyServer

*/
