#ifndef _ACTRIE_ACDAT_H_
#define _ACTRIE_ACDAT_H_

#include "actrie.h"
#include "matcher0.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct dat_node {
  size_t base;
  size_t check;
  union {
    size_t next; /* next free node */
    size_t failed;
  } dat_nf;
#define dat_free_next   dat_nf.next
#define dat_failed      dat_nf.failed
  union {
    size_t last; /* last free node */
    mdi_t idxlist;  /* out 表 */
  } dat_ld;
#define dat_free_last   dat_ld.last
#define dat_idxlist     dat_ld.idxlist
} dat_node_s, *dat_node_t;

typedef struct datrie {
  struct _matcher header;

  dat_node_s *_nodepool[REGION_SIZE];
  dat_node_t _sentinel; /* maintain free list */
  dat_node_t root;
  match_dict_t _dict;
} datrie_s, *datrie_t;

typedef struct dat_context {
  struct _context header;

  datrie_t trie;

  dat_node_t _matched;
  aobj _list;
  size_t _iCursor;
  size_t _i;

} dat_context_s, *dat_context_t;

extern const matcher_func_l dat_matcher_func;
extern const context_func_l dat_context_func;
extern const context_func_l acdat_context_func;

datrie_t dat_construct_by_trie(trie_t origin, bool enable_automation);

datrie_t dat_construct(vocab_t vocab, bool enable_automation);
bool dat_destruct(datrie_t p);

dat_context_t dat_alloc_context(datrie_t matcher);
bool dat_free_context(dat_context_t context);
bool dat_reset_context(dat_context_t context,
                       unsigned char content[],
                       size_t len);

bool dat_next_on_index(dat_context_t ctx);

bool dat_ac_next_on_node(dat_context_t ctx);

bool dat_ac_next_on_index(dat_context_t ctx);

bool dat_prefix_next_on_index(dat_context_t ctx);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ACTRIE_ACDAT_H_ */
