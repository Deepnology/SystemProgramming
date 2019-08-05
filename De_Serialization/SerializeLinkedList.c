#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h> //memcpy, memmove, strcpy
#include "Serialize.h"
#include "Sentinel.h"


struct list_node_t;

struct list_t
{
	struct list_node_t * head;
};
struct list_node_t
{
	void * data; //void* is the generic way to store generic user data
	struct list_node_t * next;
};

void serialize_list_node_t(struct list_node_t * obj, ser_buff_t * b, void(*serialize_fn_ptr)(void*, ser_buff_t*));
void serialize_list_t(struct list_t * obj, ser_buff_t * b, void(*serialize_fn_ptr)(void*, ser_buff_t*))
{
	SENTINEL_INSERTION_CODE(obj, b);
	serialize_list_node_t(obj->head, b, serialize_fn_ptr);
}
void serialize_list_node_t(struct list_node_t * obj, ser_buff_t * b, void(*serialize_fn_ptr)(void*, ser_buff_t*))
{
	SENTINEL_INSERTION_CODE(obj, b);
	serialize_fn_ptr(obj->data, b); //invoke generic user defined function ptr to serialize generic user data
	serialize_list_node_t(obj->next, b, serialize_fn_ptr);
}

struct list_node_t * de_serialize_list_node_t(ser_buff_t * b, void*(*de_serialize_fn_ptr)(ser_buff_t*));
struct list_t * de_serialize_list_t(ser_buff_t * b, void*(*de_serialize_fn_ptr)(ser_buff_t*))
{
	SENTINEL_DETECTION_CODE(b);
	struct list_t * obj = calloc(1, sizeof(struct list_t));
	obj->head = de_serialize_list_node_t(b, de_serialize_fn_ptr);
	return obj;
}
struct list_node_t * de_serialize_list_node_t(ser_buff_t * b, void*(*de_serialize_fn_ptr)(ser_buff_t*))
{
	SENTINEL_DETECTION_CODE(b);
	struct list_node_t * obj = calloc(1, sizeof(struct list_node_t));
	obj->data = de_serialize_fn_ptr(b); //invoke generic user defined function ptr to de_serialize generic user data
	obj->next = de_serialize_list_node_t(b, de_serialize_fn_ptr);
	return obj;
}

void del_list_node_t_recur(struct list_node_t * head)
{
	if (head == NULL) return;
	del_list_node_t_recur(head->next);
	free(head->data);
	free(head);
	head = NULL;
}
void del_list_t(struct list_t * list)
{
	del_list_node_t_recur(list->head);
	free(list);
	list = NULL;
}

void print_list_node_t_recur(struct list_node_t * head)
{
	if (head == NULL) return;
	printf("node: %s\n",(char*)head->data);
	print_list_node_t_recur(head->next);
}
void print_list_t(struct list_t * list)
{
	print_list_node_t_recur(list->head);
}

//now assume list_node_t::data is char*
#define LIST_NODE_STR_DATA_SIZE 128
void serialize_list_t_str_data(void * obj, ser_buff_t * b) //user defined function ptr to serialize list_node_t::data which is char*
{
	serialize_data(b, (char*)obj, LIST_NODE_STR_DATA_SIZE);
}
void * de_serialize_list_t_str_data(ser_buff_t * b) //user defined function ptr to de_serialize list_node_t::data which is char*
{
	char * data = calloc(1, LIST_NODE_STR_DATA_SIZE);
	de_serialize_data(data, b, LIST_NODE_STR_DATA_SIZE);
	return (void*)data;
}

int main(int argc, char * argv[])
{
	printf("sizeof list_t, list_node_t: %d, %d\n", sizeof(struct list_t), sizeof(struct list_node_t));

	struct list_t * list = (struct list_t*)calloc(1, sizeof(struct list_t));
	list->head = (struct list_node_t*)calloc(1, sizeof(struct list_node_t));
	list->head->data = (char*)calloc(1, LIST_NODE_STR_DATA_SIZE);
	strncpy(list->head->data, "head", strlen("head"));
	list->head->next = (struct list_node_t*)calloc(1, sizeof(struct list_node_t));
	list->head->next->data = (char*)calloc(1, LIST_NODE_STR_DATA_SIZE);
	strncpy(list->head->next->data, "node1", strlen("node1"));
	list->head->next->next = (struct list_node_t*)calloc(1, sizeof(struct list_node_t));
	list->head->next->next->data = (char*)calloc(1, LIST_NODE_STR_DATA_SIZE);
	strncpy(list->head->next->next->data, "node2", strlen("node2"));
	list->head->next->next->next = NULL;
	
	print_list_t(list);

	ser_buff_t * b;
	init_serialize_buffer(&b);
	serialize_list_t(list, b, serialize_list_t_str_data);

	//receive ....
	
	reset_serialize_buffer(b); //reset the buffer to read/write from beginning again
	struct list_t * list2 = de_serialize_list_t(b, de_serialize_list_t_str_data);

	print_list_t(list2);

	free_serialize_buffer(b);
	b = NULL;

	del_list_t(list);
	del_list_t(list2);
	return 0;
}
/*
gcc -g -c SerializeLinkedList.c -o SerializeLinkedList.o
gcc -g -c Serialize.c -o Serialize.o
gcc -g SerializeLinkedList.o Serialize.o -o SerializeLinkedList
valgrind --leak-check=full ./SerializeLinkedList
 *
 */

