//
// Created by james on 6/16/17.
//

#ifndef _ACTRIE_MATCHER_H_
#define _ACTRIE_MATCHER_H_

#include <dict.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// matcher API
// ==============

typedef enum matcher_type {
  matcher_type_stub = 0,
  matcher_type_wordattr,
  matcher_type_alteration,
  matcher_type_dat,
  matcher_type_acdat,
  matcher_type_seg_acdat,
  matcher_type_prefix_acdat,
  matcher_type_ambi,
  matcher_type_dist,
  matcher_type_ultimate
} matcher_type_e;

struct _matcher;
typedef struct _matcher *matcher_t;

struct _context;
typedef struct _context *context_t;

// base interface
//========================

matcher_t matcher_construct_by_file(matcher_type_e type, const char *path);
matcher_t matcher_construct_by_string(matcher_type_e type, const char *string);

bool matcher_destruct(matcher_t matcher);

context_t matcher_alloc_context(matcher_t matcher);
bool matcher_free_context(context_t context);

bool matcher_reset_context(context_t context, char content[], size_t len);
bool matcher_next(context_t context);

mdi_t matcher_matched_index(context_t context);
strpos_s matcher_matched_pos(context_t context);

// utils
//========================

typedef struct matched_index_pos {
  const char *keyword;
  const char *extra;
  size_t length, wlen;
  size_t so, eo;
} idx_pos_s;

idx_pos_s *matcher_remaining_matched(context_t context, size_t *out_len);
idx_pos_s *matcher_match_all(context_t context, char *content, size_t len,
                             size_t *out_len);
idx_pos_s *matcher_match_with_sort(context_t context, char *content,
                                   size_t len, size_t *out_len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ACTRIE_MATCHER_H_ */
