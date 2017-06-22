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
  matcher_type_acdat,
  matcher_type_size
} matcher_type;

struct _matcher;
typedef struct _matcher *matcher_t;

struct _context;
typedef struct _context *context_t;

typedef bool(*matcher_destruct_func)(matcher_t);
typedef context_t(*matcher_alloc_context_func)(matcher_t);

typedef struct _matcher_func {
  matcher_destruct_func destruct;
  matcher_alloc_context_func alloc_context;
} matcher_func;

struct _matcher {
  matcher_type _type;
  matcher_func _func;
};

typedef bool(*matcher_free_context_func)(context_t);
typedef bool(*matcher_reset_context_func)(context_t, char[], size_t);
typedef bool(*matcher_next_func)(context_t);

typedef struct _context_func {
  matcher_free_context_func free_context;
  matcher_reset_context_func reset_context;
  matcher_next_func next;
} context_func;

struct _context {
  matcher_type _type;
  context_func _func;

  unsigned char *content;
  size_t len;

  match_dict_index_ptr out_matched_index;
  size_t out_e;
};

matcher_t matcher_construct_by_file(matcher_type type, const char *path);
matcher_t matcher_construct_by_string(matcher_type type, const char *string);

bool matcher_destruct(matcher_t matcher);

context_t matcher_alloc_context(matcher_t matcher);
bool matcher_free_context(context_t context);

bool matcher_reset_context(context_t context, char content[], size_t len);
bool matcher_next(context_t context);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MATCHER_H_ */
