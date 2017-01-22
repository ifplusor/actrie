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


enum match_dict_keyword_type {
	match_dict_keyword_type_normal = 0,
	match_dict_keyword_type_alpha_number = 1,
	match_dict_keyword_type_empty = 2
};

typedef struct match_dict_index {
	struct match_dict_index *next;
	const char *keyword;
	const char *extra;
	enum match_dict_keyword_type type;
} match_dict_index, *match_dict_index_ptr;

typedef bool (*dict_parser_callback)(match_dict_index_ptr index, void *argv[]);

typedef struct match_dict {
	match_dict_index_ptr index;
	size_t count;

	char *buff;
	size_t size;
	size_t _cursor;
	size_t _out_key_cur, _out_extra_cur;
	enum match_dict_keyword_type _out_type;

	int _ref_count;
} match_dict, *match_dict_ptr;


extern const size_t POOL_REGION_SIZE;
extern const size_t POOL_POSITION_SIZE;

extern const char *ALLOCATED_FLAG;

extern const bool alpha_number_bitmap[256];


long long system_millisecond();

match_dict_ptr dict_alloc();

match_dict_ptr dict_assign(match_dict_ptr dict);

void dict_release(match_dict_ptr dict);

bool dict_parser_by_file(FILE *fp, match_dict_ptr dict,
						 dict_parser_callback callback, void *argv[]);

bool dict_parser_by_s(const char *s, match_dict_ptr dict,
					  dict_parser_callback callback, void *argv[]);

#endif // _MATCH_COMMON_H_
