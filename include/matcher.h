//
// Created by james on 6/16/17.
//

#ifndef _MATCHER_H_
#define _MATCHER_H_


#include "dict.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


// API
// ==============

typedef enum matcher_type {
	matcher_type_dat = 0,
	matcher_type_acdat = 1,
} matcher_type;

struct _matcher;
typedef struct _matcher *matcher_t;

struct _context;
typedef struct _context *context_t;

matcher_t matcher_construct_by_file(matcher_type type, const char *path);
matcher_t matcher_construct_by_string(matcher_type type, const char *string);

bool matcher_destruct(matcher_t matcher);

context_t matcher_alloc_context(matcher_t matcher);
bool matcher_free_context(context_t context);

bool matcher_reset_context(context_t context, char content[], size_t len);
bool matcher_next(context_t context);


// interface
// ==============

struct _matcher {
	void *_self;
	matcher_type _type;
};

struct _context {
	void *_self;
	matcher_type _type;

	unsigned char   *content;
	size_t          len;

	match_dict_index_ptr out_matched_index;
	size_t out_e;
};


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MATCHER_H_ */
