#ifndef _ACTRIE_ACDAT_H_
#define _ACTRIE_ACDAT_H_

#include "actrie.h"
#include "matcher0.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct dat_node {
  size_t check;
  size_t base;
  size_t failed;
  aobj idxlist;  /* out 表 */
#define dat_free_next base   /* next free node */
#define dat_free_last failed /* last free node */
} dat_node_s, *dat_node_t;

typedef struct datrie {
  matcher_s hdr;

  dat_node_s *_nodepool[REGION_SIZE];
  dat_node_t _sentinel; /* maintain free list */
  dat_node_t root;
  match_dict_t _dict;
} datrie_s, *datrie_t;

typedef struct dat_context {
  context_s hdr;

  datrie_t trie;

  dat_node_t _matched;
  aobj _list;
  size_t _iCursor;
  size_t _i;
  size_t _e;

} dat_context_s, *dat_context_t;

matcher_conf_t dat_matcher_conf(uint8_t id, matcher_type_e type, matcher_conf_t stub);

datrie_t dat_construct_by_trie(trie_t origin, bool enable_automation);
matcher_t dat_construct(match_dict_t dict, matcher_conf_t conf);
bool dat_destruct(datrie_t p);

dat_context_t dat_alloc_context(datrie_t matcher);
bool dat_free_context(dat_context_t context);
bool dat_reset_context(dat_context_t context, char content[], size_t len);

bool dat_match_end(dat_context_t ctx);

bool dat_next_on_node(dat_context_t ctx);
bool dat_next_on_index(dat_context_t ctx);
bool dat_ac_next_on_node(dat_context_t ctx);
bool dat_ac_next_on_index(dat_context_t ctx);
bool dat_seg_ac_next_on_index(dat_context_t ctx);
bool dat_prefix_next_on_index(dat_context_t ctx);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ACTRIE_ACDAT_H_ */
