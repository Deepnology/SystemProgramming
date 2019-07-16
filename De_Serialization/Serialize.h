#ifndef __SERIALIZE__
#define __SERIALIZE__

typedef struct serialize_buffer
{
	#define SERIALIZE_BUFFER_DEFAULT_SIZE 512
	void * b;
	int size;
	int next;

} ser_buff_t;

/*init functions*/
void init_serialize_buffer(ser_buff_t **b);
void init_serialize_buffer_of_defined_size(ser_buff_t **b, int size);

/*get functions*/
char is_serialize_buffer_empty(ser_buff_t *b);
int get_serialize_buffer_data_size(ser_buff_t *b); //total data size
int get_serialize_buffer_size(ser_buff_t *b); //total buffer size
int get_serialize_buffer_current_ptr_offset(ser_buff_t *b);
char * get_serialize_buffer_current_ptr(ser_buff_t *b);

void serialize_buffer_skip(ser_buff_t *b, int size);
void copy_in_serialize_buffer_by_offset(ser_buff_t *b, int size, char *value, int offset);

/*de-serialize functions*/
void serialize_data(ser_buff_t *b, char *data, int val_size);
void de_serialize_data(char *dest, ser_buff_t *b, int val_size);

/*free resources*/
void free_serialize_buffer(ser_buff_t *b);

/*reset functions*/
void truncate_serialize_buffer(ser_buff_t **b);
void reset_serialize_buffer(ser_buff_t *b);

/*details*/
void print_serialize_buffer_details(ser_buff_t *b, const char *fn, int lineno);

#endif
