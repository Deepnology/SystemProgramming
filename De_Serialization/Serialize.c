#include <stdio.h>
#include "Serialize.h"
#include <memory.h>
#include <stdlib.h>
#include <assert.h>

void init_serialize_buffer(ser_buff_t **b)
{
	(*b) = (ser_buff_t*)calloc(1, sizeof(ser_buff_t));
	(*b)->b = calloc(1, SERIALIZE_BUFFER_DEFAULT_SIZE);
	(*b)->size = SERIALIZE_BUFFER_DEFAULT_SIZE;
	(*b)->next = 0;
}
void init_serialize_buffer_of_defined_size(ser_buff_t **b, int size)
{
	(*b) = (ser_buff_t*)calloc(1, sizeof(ser_buff_t));
	(*b)->b = calloc(1, size);
	(*b)->size = size;
	(*b)->next = 0;
}


char is_serialize_buffer_empty(ser_buff_t *b)
{
	if (b->next == 0) return 1;
	return 0;
}
int get_serialize_buffer_data_size(ser_buff_t *b) //total data size
{
	return b->next;
}
int get_serialize_buffer_size(ser_buff_t *b) //total buffer size
{
	return b->size;
}
int get_serialize_buffer_current_ptr_offset(ser_buff_t *b)
{
	if (!b) return -1;
	return b->next;
}
char * get_serialize_buffer_current_ptr(ser_buff_t *b)
{
	if (!b) return NULL;
	return b->b + b->next;
}


void serialize_buffer_skip(ser_buff_t *b, int size)
{
	int available_size = b->size - b->next;
	if (available_size >= size)
	{
		b->next += size;
		return;
	}

	while (available_size < size)
	{
		b->size = b->size * 2;
		available_size = b->size - b->next;
	}

	//now available_size >= size
	b->b = realloc(b->b, b->size);
	b->next += size;
}
void copy_in_serialize_buffer_by_offset(ser_buff_t *b, int size, char *value, int offset)
{
	if (offset > b->size)
	{
		printf("%s(): Error: Attempt to write outside buffer boundaries\n", __FUNCTION__);
		return;
	}

	memcpy(b->b + offset, value, size);
}


void serialize_data(ser_buff_t *b, char *data, int nbytes)
{
	if (!b) assert(0);

	int available_size = b->size - b->next;
	char isResize = 0;

	while (available_size < nbytes)
	{
		b->size = b->size * 2;
		available_size = b->size - b->next;
		isResize = 1;
	}

	if (isResize)
		b->b = realloc(b->b, b->size);

	memcpy((char*)b->b + b->next, data, nbytes);
	b->next += nbytes;
}
void de_serialize_data(char *dest, ser_buff_t *b, int size)
{
	if (!b || !b->b) assert(0);
	if (!size) return;
	if ((b->size - b->next) < size) assert(0);

	memcpy(dest, b->b + b->next, size);
	b->next += size;
}


void free_serialize_buffer(ser_buff_t *b)
{
	free(b->b);
	free(b);
}


void truncate_serialize_buffer(ser_buff_t **b) //remove the spaces in (*b)->b[(*b)->next:(*b)->size]
{
	if ((*b)->next == (*b)->size) return;

	ser_buff_t *clone = NULL;
	init_serialize_buffer_of_defined_size(&clone, (*b)->next);
	memcpy(clone->b, (*b)->b, (*b)->next);
	clone->next = clone->size;
	free_serialize_buffer(*b);
	*b = clone;
}
void reset_serialize_buffer(ser_buff_t *b) //reset the buffer to read/write it from beginning again
{
	b->next = 0;
}


void print_serialize_buffer_details(ser_buff_t *b, const char *fn, int lineno)
{
	printf("%s():%d : starting address = 0x%x\n", fn, lineno, (unsigned int)b);
	printf("size = %d\n", b->size);
	printf("next = %d\n", b->next);
}
