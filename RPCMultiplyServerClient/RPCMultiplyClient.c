#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "RPCCommon.h"
#include "../De_Serialization/Serialize.h"


ser_buff_t * multiply_client_stub_marshal(int a, int b) //serialize
{
	ser_buff_t * client_send_ser_buff = NULL;
	init_serialize_buffer_of_defined_size(&client_send_ser_buff, MAX_RECV_SEND_BUFF_SIZE);

	serialize_buffer_skip(client_send_ser_buff, sizeof(rpc_hdr_t)); //reserve rpc_hdr size, note: since rpc_hdr_t contains only 2 uint, the sizeof operator will not pad extra bytes

	rpc_hdr_t rpc_hdr;
	rpc_hdr.op_id = RPC_MULTIPLY_ID;
	rpc_hdr.payload_size = 0;

	serialize_data(client_send_ser_buff, (char*)&a, sizeof(int));
	serialize_data(client_send_ser_buff, (char*)&b, sizeof(int));

	rpc_hdr.payload_size = get_serialize_buffer_data_size(client_send_ser_buff) - sizeof(rpc_hdr_t);

	copy_in_serialize_buffer_by_offset(client_send_ser_buff, sizeof(rpc_hdr.op_id), (char*)&rpc_hdr.op_id, 0);
	copy_in_serialize_buffer_by_offset(client_send_ser_buff, sizeof(rpc_hdr.payload_size), (char*)&rpc_hdr.payload_size, sizeof(rpc_hdr.op_id));
	return client_send_ser_buff;
}
int multiply_client_stub_unmarshal(ser_buff_t * client_recv_ser_buff) //de-serialize
{
	int res = 0;
	de_serialize_data((char*)&res, client_recv_ser_buff, sizeof(int));
	return res;
}

void rpc_send_recv(ser_buff_t * client_send_ser_buff, ser_buff_t * client_recv_ser_buff) //UDP socket sendto recvfrom
{
	int clientSocketFD = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	printf("Socket: %d\n", clientSocketFD);
	if (clientSocketFD == -1)
	{
		perror("Socket");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(SERVER_PORT);
	struct hostent * host = (struct hostent*)gethostbyname(SERVER_IP);
	servAddr.sin_addr = *((struct in_addr*)host->h_addr);
	int sentSize = sendto(clientSocketFD, client_send_ser_buff->b, get_serialize_buffer_data_size(client_send_ser_buff),
			0, (struct sockaddr*)&servAddr, sizeof(struct sockaddr));
	printf("Send: %d\n", sentSize);
	if (sentSize == -1)
	{
		perror("Sendto");
		exit(EXIT_FAILURE);
	}

	unsigned int servAddrLen = sizeof(struct sockaddr); //this must be specified !
	int recvSize = recvfrom(clientSocketFD, client_recv_ser_buff->b, get_serialize_buffer_size(client_recv_ser_buff),
			0, (struct sockaddr*)&servAddr, &servAddrLen);
	printf("Recv: %d\n", recvSize);
	if (recvSize == -1)
	{
		perror("Recvfrom");
		exit(EXIT_FAILURE);
	}
}

int multiply_rpc(int a, int b)
{
	ser_buff_t * client_send_ser_buff = NULL;
	client_send_ser_buff = multiply_client_stub_marshal(a, b); //serialize
	ser_buff_t * client_recv_ser_buff = NULL;
	init_serialize_buffer_of_defined_size(&client_recv_ser_buff, MAX_RECV_SEND_BUFF_SIZE);

	rpc_send_recv(client_send_ser_buff, client_recv_ser_buff); //UDP socket sendto recvfrom
	free_serialize_buffer(client_send_ser_buff);
	client_send_ser_buff = NULL;

	int res = multiply_client_stub_unmarshal(client_recv_ser_buff); //de-serialize
	free_serialize_buffer(client_recv_ser_buff);
	return res;
}

int main(int argc, char * argv[])
{
	int a, b;
	printf("Enter 1st number: ");
	scanf("%d", &a);
	printf("Enter 2nd number: ");
	scanf("%d", &b);

	int res = multiply_rpc(a, b);

	printf("%d * %d = %d\n", a, b, res);
	return 0;
}



