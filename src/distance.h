//
// Created by james on 6/19/17.
//

#ifndef _MATCH_DISTANCE_H_
#define _MATCH_DISTANCE_H_

#include "dict0.h"
#include "matcher0.h"

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

  match_dict_index_s out_index;             /* will change when call 'next' */
#ifdef REPLACE_BY_ZERO
  unsigned char _c;
#endif

  size_t *_utf8_pos;

  struct _context _history_context[HISTORY_SIZE];          /* cycle queue */
  size_t _hcnt, _htidx, _i;             /* count, tail, index for history */

  context_t _head_context, _tail_context, _digit_context;
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

#endif /* _MATCH_DISTANCE_H_ */
