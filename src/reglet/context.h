/**
 * context.h
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#ifndef __ACTRIE_REGEX_CONTEXT_H__
#define __ACTRIE_REGEX_CONTEXT_H__

#include <alib/collections/list/deque.h>
#include <alib/collections/list/prique.h>
#include <alib/collections/map/avl.h>
#include <alib/memory/dynapool.h>
#include <alib/object/dstr.h>
#include <alib/string/astr.h>
#include <alib/string/utf8.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _position_cache_node_ {
  strpos_s pos;
  union {
    avl_node_s avl_elem;
    deque_node_s deque_elem;
    dstr_t extra;
  } embed;
} pos_cache_s, *pos_cache_t;

sptr_t pos_cache_cmp_eoso(avl_node_t node, void* key);
sptr_t pos_cache_cmp_soeo(avl_node_t node, void* key);
sptr_t pos_cache_eq_eo(avl_node_t node, void* key);

sptr_t pos_cache_so_in_range(avl_node_t node, void* arg);
sptr_t pos_cache_eo_in_range(avl_node_t node, void* arg);

sptr_t pos_cache_so_in_word(avl_node_t node, void* arg);
sptr_t pos_cache_eo_in_word(avl_node_t node, void* arg);

void free_pos_cache(avl_node_t node, void* arg);

typedef size_t (*fix_pos_f)(size_t pos, size_t diff, bool plus_or_subtract, void* arg);

typedef struct _regex_context_ {
  dynapool_t pos_cache_pool;
  avl_t expr_ctx_map;
  prique_t output_queue;
  deque_node_s ambi_queue[1];
  fix_pos_f fix_pos_func;
  void* fix_pos_arg;
  bool reset_or_free;
} reg_ctx_s, *reg_ctx_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  // __ACTRIE_REGEX_CONTEXT_H__
