//
// Created by james on 9/25/17.
//

#ifndef _MATCH_MATCHER0_H_
#define _MATCH_MATCHER0_H_

#include <matcher.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

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

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // _MATCH_MATCHER0_H_
