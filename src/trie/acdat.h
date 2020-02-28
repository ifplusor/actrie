/**
 * acdat.h - Double-Array Trie
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#ifndef __ACTRIE_ACDAT_H__
#define __ACTRIE_ACDAT_H__

#include "actrie.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _datrie_node_ {
  size_t check;
  size_t base;
  size_t failed;
  void* value;
#define dat_free_next base   /* next free node */
#define dat_free_last failed /* last free node */
} dat_node_s, *dat_node_t;

typedef struct _datrie_ {
  segarray_t node_array;
  dat_node_t _sentinel; /* maintain free list */
  dat_node_t root;
} dat_s, *dat_t;

typedef struct _datrie_context_ {
  strlen_s content;

  dat_t trie;

  dat_node_t _matched;
  aobj _value;
  size_t _iCursor;
  size_t _i;
  size_t _e;
} dat_ctx_s, *dat_ctx_t;

typedef void (*dat_node_free_f)(dat_t dat, void* node);

dat_t dat_construct_by_trie(trie_t origin, bool enable_automation);
void dat_destruct(dat_t datrie, dat_node_free_f node_free_func);

dat_ctx_t dat_alloc_context(dat_t datrie);
bool dat_free_context(dat_ctx_t context);
bool dat_reset_context(dat_ctx_t context, char content[], size_t len);

bool dat_match_end(dat_ctx_t ctx);

bool dat_next_on_node(dat_ctx_t ctx);
bool dat_ac_next_on_node(dat_ctx_t ctx);
bool dat_prefix_next_on_node(dat_ctx_t ctx);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  // __ACTRIE_ACDAT_H__
