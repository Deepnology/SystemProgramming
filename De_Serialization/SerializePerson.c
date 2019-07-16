#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "Person_t.h"
#include "Serialize.h"
#include "Sentinel.h"

void serialize_person_t(person_t *obj, ser_buff_t *b);
void serialize_company_t(company_t *obj, ser_buff_t *b);
company_t * de_serialize_company_t(ser_buff_t *b);
person_t * de_serialize_person_t(ser_buff_t *b);
void print_person(person_t *obj);
void print_company(company_t *obj);

company_t * de_serialize_company_t(ser_buff_t *b)
{
	SENTINEL_DETECTION_CODE(b);

	company_t * obj = calloc(1, sizeof(company_t));

	de_serialize_data((char*)obj->comp_name, b, 32);
	de_serialize_data((char*)&obj->emp_strength, b, sizeof(int));
	obj->CEO = de_serialize_person_t(b);
	return obj;
}
person_t * de_serialize_person_t(ser_buff_t *b)
{
	SENTINEL_DETECTION_CODE(b);

	person_t * obj = calloc(1, sizeof(person_t));

	for (int i = 0; i < 4; ++i)
		de_serialize_data((char*)&obj->vehicle_nos[i], b, sizeof(unsigned int));

	de_serialize_data((char*)&obj->age, b, sizeof(int));

	unsigned int sentinel = 0;
	de_serialize_data((char*)&sentinel, b, sizeof(unsigned int));
	if (sentinel == 0xFFFFFFFF)
		obj->height = NULL;
	else
	{
		serialize_buffer_skip(b, -1 * sizeof(unsigned int));
		obj->height = calloc(1, sizeof(int));
		de_serialize_data((char*)obj->height, b, sizeof(int));
	}

	for (int i = 0; i < 5; ++i)
	{
		de_serialize_data((char*)&sentinel, b, sizeof(unsigned int));
		if (sentinel == 0xFFFFFFFF)
			obj->last_salary_amounts[i] = NULL;
		else
		{
			serialize_buffer_skip(b, -1 * sizeof(unsigned int));
			obj->last_salary_amounts[i] = calloc(1, sizeof(unsigned int));
			de_serialize_data((char*)obj->last_salary_amounts[i], b, sizeof(unsigned int));
		}
	}

	de_serialize_data((char*)obj->name, b, 32);
	
	company_t * company = de_serialize_company_t(b);
	obj->company = *company; //shallow copy because obj->company is not a pointer
	free(company); //shallow free

	for (int i = 0; i < 3; ++i)
	{
		company = de_serialize_company_t(b);
		obj->dream_companies[i] = *company; //shallow copy
		free(company); //shallow free
	}

	obj->CEO = de_serialize_person_t(b);

	for (int i = 0; i < 5; ++i)
		obj->administrative_staff[i] = de_serialize_person_t(b);

	return obj;
}
void serialize_company_t(company_t *obj, ser_buff_t *b)
{
	SENTINEL_INSERTION_CODE(obj, b);

	serialize_data(b, (char*)obj->comp_name, 32);
	serialize_data(b, (char*)&obj->emp_strength, sizeof(int));
	serialize_person_t(obj->CEO, b);
}
void serialize_person_t(person_t *obj, ser_buff_t *b)
{
	SENTINEL_INSERTION_CODE(obj, b);

	for (int i = 0; i < 4; ++i)
		serialize_data(b, (char*)&obj->vehicle_nos[i], sizeof(unsigned int));

	serialize_data(b, (char*)&obj->age, sizeof(unsigned int));
	
	unsigned int sentinel = 0xFFFFFFFF;
	if (obj->height)
		serialize_data(b, (char*)obj->height, sizeof(int));
	else
		serialize_data(b, (char*)&sentinel, sizeof(unsigned int));

	for (int i = 0; i < 5; ++i)
		if (obj->last_salary_amounts[i])
			serialize_data(b, (char*)obj->last_salary_amounts[i], sizeof(unsigned int));
		else
			serialize_data(b, (char*)&sentinel, sizeof(unsigned int));

	serialize_data(b, (char*)obj->name, 32);

	serialize_company_t(&obj->company, b);

	for (int i = 0; i < 3; ++i)
		serialize_company_t(&obj->dream_companies[i], b);

	serialize_person_t(obj->CEO, b);

	for (int i = 0; i < 5; ++i)
		serialize_person_t(obj->administrative_staff[i], b);
}

void print_company(company_t *obj)
{
	if (!obj) return;
	printf("comp_name = %s\n", obj->comp_name);
	printf("emp_strength = %d\n", obj->emp_strength);
	print_person(obj->CEO);
}
void print_person(person_t *obj)
{
	if (!obj) return;
	printf("=== person_t: 0x%x ===\n", (unsigned int)obj);
	for (int i = 0; i < 4; ++i)
		printf("vehicle no[%d] = %d\n", i, obj->vehicle_nos[i]);
	printf("age = %d\n", obj->age);
	if (obj->height)
		printf("height = %d\n", *obj->height);
	else
		printf("height = NULL\n");
	for (int i = 0; i < 5; ++i)
	{
		if (obj->last_salary_amounts[i])
			printf("last_salary_amounts[%d] = %d\n", i, *obj->last_salary_amounts[i]);
		else
			printf("last_salary_amounts[%d] = NULL\n", i);
	}
	printf("name = %s\n", obj->name);
	print_company(&obj->company);
	for (int i = 0; i < 3; ++i)
		print_company(&obj->dream_companies[i]);
	print_person(obj->CEO);
	for (int i = 0; i < 5; ++i)
		print_person(obj->administrative_staff[i]);
}

int main(int argc, char **argv)
{
	person_t p1;
	memset(&p1, 0, sizeof(person_t)); //note: sizeof(person_t) will return the size in bytes after adding padded bytes by compiler !!!
	p1.vehicle_nos[0] = 1000;
	p1.vehicle_nos[1] = 1001;
	p1.vehicle_nos[2] = 1002;
	p1.vehicle_nos[3] = 1003;
	p1.age = 31;
	p1.height = calloc(1, sizeof(int));
	*p1.height = 167;
	p1.last_salary_amounts[0] = NULL;
	p1.last_salary_amounts[1] = NULL;
	p1.last_salary_amounts[2] = calloc(1, sizeof(unsigned int));
	*p1.last_salary_amounts[2] = 20000;
	p1.last_salary_amounts[3] = calloc(1, sizeof(unsigned int));
	*p1.last_salary_amounts[3] = 40000;
	p1.last_salary_amounts[4] = NULL;
	strncpy(p1.name, "Abhishek", strlen("Abhishek"));
	strncpy(p1.company.comp_name, "Juniper", strlen("Juniper"));
	p1.company.emp_strength = 10000;
	p1.company.CEO = NULL;

	print_person(&p1);

	ser_buff_t *b;
	init_serialize_buffer(&b);
	serialize_person_t(&p1, b);

	//receive ...

	reset_serialize_buffer(b); //reset the buffer to read/write it from beginning agian
	person_t * p2 = de_serialize_person_t(b);

	free_serialize_buffer(b);
	b = NULL;

	print_person(p2);

	//free p2
	
	return 0;
}

/*
gcc -g -c SerializePerson.c -o SerializePerson.o
gcc -g -c Serialize.c -o Serialize.o
gcc -g SerializePerson.o Serialize.o -o SerializePerson
 * 
 */
