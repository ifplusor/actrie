//
// Created by james on 9/25/17.
//

#ifndef _ACTRIE_MATCHER0_H_
#define _ACTRIE_MATCHER0_H_

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
} matcher_func_l, *matcher_func_t;

typedef struct _matcher {
  matcher_type_e _type;
  matcher_func_l _func;
} matcher_s;

typedef bool(*matcher_free_context_func)(context_t);
typedef bool(*matcher_reset_context_func)(context_t, char[], size_t);
typedef bool(*matcher_next_func)(context_t);

/* context 成员函数 */
typedef struct _context_func {
  matcher_free_context_func free_context;  /* 销毁 */
  matcher_reset_context_func reset_context;  /* 初始化 */
  matcher_next_func next;  /* 迭代匹配 */
} context_func_l, *context_func_t;

typedef struct _context {
  matcher_type_e _type;
  context_func_t _func;

  unsigned char *content;
  size_t len;

  mdi_t out_matched_index;  /* volatile, need deep copy */
  size_t out_eo;               /* end offset of current matched word */
} context_s;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // _ACTRIE_MATCHER0_H_
