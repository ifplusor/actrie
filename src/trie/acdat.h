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

struct _datrie_node_;
typedef struct _datrie_node_ dat_node_s;
typedef dat_node_s* dat_node_t;

typedef union _datrie_index_or_ptr_ {
  size_t idx;
  dat_node_t ptr;
} dat_idxptr_t;

struct _datrie_value_;
typedef struct _datrie_value_ dat_value_s;
typedef dat_value_s* dat_value_t;

struct _datrie_value_ {
  void* value;
  dat_value_t next;
};

struct _datrie_node_ {
  dat_idxptr_t check;
  dat_idxptr_t base;
  dat_idxptr_t failed;
  union {
    void* raw;
    void* linked;
  } value;
#define dat_free_next base.idx   /* next free node */
#define dat_free_last failed.idx /* last free node */
};

typedef struct _datrie_ {
  segarray_t node_array;
  dat_node_t _sentinel; /* maintain free list */
  dat_node_t root;
  segarray_t value_array;
  bool enable_automation;
} dat_s, *dat_t;

typedef struct _datrie_context_ {
  strlen_s content;

  dat_t trie;

  union {
    dat_node_t node;
    dat_value_t value;
  } _matched;
  dat_node_t _cursor;
  size_t _begin;
  size_t _read;
} dat_ctx_s, *dat_ctx_t;

typedef void (*dat_node_free_f)(dat_t dat, void* node);

dat_t dat_construct_by_trie(trie_t origin, bool enable_automation);
void dat_destruct(dat_t datrie, dat_node_free_f node_free_func);

dat_ctx_t dat_alloc_context(dat_t datrie);
bool dat_free_context(dat_ctx_t context);
void dat_reset_context(dat_ctx_t context, char content[], size_t len);

bool dat_match_end(dat_ctx_t ctx);

static inline void* dat_matched_value(dat_ctx_t ctx) {
  if (ctx->trie->enable_automation) {
    return ctx->_matched.value->value;
  } else {
    return ctx->_matched.node->value.raw;
  }
}

bool dat_next_on_node(dat_ctx_t ctx);
bool dat_prefix_next_on_node(dat_ctx_t ctx);

bool dat_ac_next_on_node(dat_ctx_t ctx);
bool dat_ac_prefix_next_on_node(dat_ctx_t ctx);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  // __ACTRIE_ACDAT_H__
