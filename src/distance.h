//
// Created by james on 6/19/17.
//

#ifndef _MATCH_DISTANCE_H_
#define _MATCH_DISTANCE_H_

#include "matcher.h"

#define HISTORY_SIZE 50000

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct dist_matcher {
  struct _matcher header;

  matcher_t _head_matcher, _tail_matcher;
  match_dict_ptr _dict;
} *dist_matcher_t;

typedef enum dist_match_state {
  dist_match_state_new_round = 0,
  dist_match_state_check_history,
  dist_match_state_check_tail
} dist_match_state;

typedef struct dist_context {
  struct _context header;

  dist_matcher_t _matcher;

  match_dict_index out_index;
  unsigned char _c;

  struct _context _history_context[HISTORY_SIZE];  /* 循环表 */
  size_t _hcnt, _hidx, _i;

  context_t _head_context, _tail_context;
  dist_match_state _state;

} *dist_context_t;

extern const matcher_func dist_matcher_func;
extern const context_func dist_context_func;

dist_matcher_t dist_construct_by_file(const char *path, bool enable_automation);
dist_matcher_t dist_construct_by_string(const char *string,
                                        bool enable_automation);
bool dist_destruct(dist_matcher_t p);

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
