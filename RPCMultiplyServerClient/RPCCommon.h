#ifndef __RPC_COMMON__
#define __RPC_COMMON__

#define MAX_RECV_SEND_BUFF_SIZE 2048

#define SERVER_PORT 2000
#define SERVER_IP "127.0.0.1"

typedef struct rpc_hdr_
{
	unsigned int op_id;
	unsigned int payload_size;
}rpc_hdr_t;

#define RPC_MULTIPLY_ID 1

#endif
