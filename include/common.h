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


typedef struct match_dict_index {
	const char *keyword;
	const char *extra;
	struct match_dict_index *next;
} match_dict_index, *match_dict_index_ptr;

typedef bool (*dict_parser_callback)(match_dict_index_ptr index, void *argv[]);

typedef struct match_dict {
	match_dict_index_ptr index;
	size_t count;

	char *buff;
	size_t size;
	size_t _cursor;
	size_t _out_key_cur, _out_extra_cur;

	int _ref_count;
} match_dict, *match_dict_ptr;


extern const size_t POOL_REGION_SIZE;
extern const size_t POOL_POSITION_SIZE;

extern const char *ALLOCATED_FLAG;

long long system_millisecond();

match_dict_ptr dict_alloc();

match_dict_ptr dict_assign(match_dict_ptr dict);

void dict_release(match_dict_ptr dict);

bool dict_parser_by_file(FILE *fp, match_dict_ptr dict,
						 dict_parser_callback callback, void *argv[]);

bool dict_parser_by_s(const char *s, match_dict_ptr dict,
					  dict_parser_callback callback, void *argv[]);

#endif // _MATCH_COMMON_H_
