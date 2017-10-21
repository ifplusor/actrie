//
// Created by james on 6/19/17.
//

#ifndef _ACTRIE_DISTANCE_H_
#define _ACTRIE_DISTANCE_H_

#include "matcher0.h"
#include "dynapool.h"
#include "dlnk.h"
#include "mdimap.h"

#define HISTORY_SIZE 50

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct dist_matcher {
  matcher_s header;

  matcher_t _head_matcher, _tail_matcher;
  match_dict_t _dict;
} *dist_matcher_t;

typedef enum dist_match_state {
  dist_match_state_new_round = 0,
  dist_match_state_check_history,
  dist_match_state_check_tail,
  dist_match_state_check_prefix,
} dist_match_state;

typedef struct dist_context {
  context_s header; /* 'header.out_matched_index' point 'out_index' */

  dist_matcher_t _matcher;

  mdi_s out_index;     /* will change when call 'next' */
#ifdef DIST_REPLACE_BY_ZERO
  unsigned char _c;
#endif

  size_t *_utf8_pos;

  dynapool_t _mdiqn_pool;
  deque_node_s _tail_cache;
  mdimap_t _tail_map;
  mdim_node_t _tail_node;
  size_t _tail_so;

  context_t _head_ctx, _tail_ctx, _digit_ctx;
  dist_match_state _state;

} *dist_context_t;

extern const matcher_func_l dist_matcher_func;
extern const context_func_l dist_context_func;

dist_matcher_t dist_construct(vocab_t vocab, bool enable_automation);
bool dist_destruct(dist_matcher_t self);

dist_context_t dist_alloc_context(dist_matcher_t matcher);
bool dist_free_context(dist_context_t context);
bool dist_reset_context(dist_context_t context,
                        unsigned char content[],
                        size_t len);

bool dist_next_on_index(dist_context_t ctx);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ACTRIE_DISTANCE_H_ */
