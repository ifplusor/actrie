//
// Created by james on 9/27/17.
//

#ifndef _MATCH_DISAMBI_H_
#define _MATCH_DISAMBI_H_

#include "acdat.h"
#include "avl.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct ambi_matcher {
  struct _matcher header;

  matcher_t _pure_matcher;
  match_dict_t _dict;
} *ambi_matcher_t;

typedef enum ambi_match_state {
  ambi_match_state_new_round = 0,
  ambi_match_state_check_history,
  ambi_match_state_check_tail,
  ambi_match_state_check_prefix,
} ambi_match_state;

typedef struct ambi_context {
  struct _context header; /* 'header.out_matched_index' point 'out_index' */

  ambi_matcher_t _matcher;

  avl_t _word_map;
  avl_t _ambi_map;

  context_t _pure_context;

} *ambi_context_t;

extern const matcher_func ambi_matcher_func;
extern const context_func ambi_context_func;

ambi_matcher_t ambi_construct_by_file(const char *path, bool enable_automation);
ambi_matcher_t ambi_construct_by_string(const char *string,
                                        bool enable_automation);
bool ambi_destruct(ambi_matcher_t self);

ambi_context_t ambi_alloc_context(ambi_matcher_t matcher);
bool ambi_free_context(ambi_context_t context);
bool ambi_reset_context(ambi_context_t context,
                        unsigned char content[],
                        size_t len);

bool ambi_next_on_index(ambi_context_t ctx);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //_MATCH_DISAMBI_H_
