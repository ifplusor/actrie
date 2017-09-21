//
// Created by james on 6/16/17.
//

#ifndef _MATCHER_H_
#define _MATCHER_H_

#include "dict.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


// matcher API
// ==============

typedef enum matcher_type {
  matcher_type_dat = 0,
  matcher_type_acdat,
  matcher_type_distance,
  matcher_type_size
} matcher_type;

struct _matcher;
typedef struct _matcher *matcher_t;

struct _context;
typedef struct _context *context_t;

typedef bool(*matcher_destruct_func)(matcher_t);
typedef context_t(*matcher_alloc_context_func)(matcher_t);

/* matcher 成员函数 */
typedef struct _matcher_func {
  matcher_destruct_func destruct;  /* 销毁 */
  matcher_alloc_context_func alloc_context;  /* 构造 context */
} matcher_func;

struct _matcher {
  matcher_type _type;
  matcher_func _func;
};

typedef bool(*matcher_free_context_func)(context_t);
typedef bool(*matcher_reset_context_func)(context_t, char[], size_t);
typedef bool(*matcher_next_func)(context_t);

/* context 成员函数 */
typedef struct _context_func {
  matcher_free_context_func free_context;  /* 销毁 */
  matcher_reset_context_func reset_context;  /* 初始化 */
  matcher_next_func next;  /* 迭代匹配 */
} context_func;

struct _context {
  matcher_type _type;
  context_func _func;

  unsigned char *content;
  size_t len;

  match_dict_index_ptr out_matched_index;  /* volatile, need deep copy */
  size_t out_e;
};

matcher_t matcher_construct_by_file(matcher_type type, const char *path);
matcher_t matcher_construct_by_string(matcher_type type, const char *string);

bool matcher_destruct(matcher_t matcher);

context_t matcher_alloc_context(matcher_t matcher);
bool matcher_free_context(context_t context);

bool matcher_reset_context(context_t context, char content[], size_t len);
bool matcher_next(context_t context);

typedef struct _idx_pos {
  const char *keyword;
  const char *extra;
  size_t length, wlen;
  size_t os, oe;
} idx_pos_s;

idx_pos_s *matcher_remaining_matched(context_t context, size_t *out_len);
idx_pos_s *matcher_match_all(context_t context, char *content, size_t len,
                             size_t *out_len);
idx_pos_s *matcher_match_with_sort(context_t context, char *content,
                                          size_t len, size_t *out_len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MATCHER_H_ */
