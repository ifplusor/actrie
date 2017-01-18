#ifndef _MATCH_COMMON_H_
#define _MATCH_COMMON_H_


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


#ifndef __bool_true_false_are_defined
#define bool int
#define true 1
#define false 0
#endif // __bool_true_false_are_defined


#define REGION_SIZE 0x00001000
#define REGION_OFFSET 18
#define POSITION_MASK 0x0003FFFF

#define MAX_LINE_SIZE 1024*1024


typedef bool (*dict_parser_callback)(const char keyword[], const char extra[],
									 void *argv[]);

typedef struct match_dict {
	char *buff;
	size_t size;
	size_t _cursor;
	size_t out_key_cur, out_extra_cur;
} match_dict, *match_dict_ptr;


extern const size_t POOL_REGION_SIZE;
extern const size_t POOL_POSITION_SIZE;

extern const char *ALLOCATED_FLAG;

long long system_millisecond();

void dict_release(match_dict_ptr dict);

bool dict_parser(FILE *fp, match_dict_ptr dict, dict_parser_callback callback,
				 void *argv[]);


#endif // _MATCH_COMMON_H_
