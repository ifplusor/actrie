//
// Created by james on 6/16/17.
//

#ifndef _MATCH_MATCHER_H_
#define _MATCH_MATCHER_H_

#include <dict.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// matcher API
// ==============

typedef enum matcher_type {
  matcher_type_dat = 0,
  matcher_type_acdat,
  matcher_type_distance,
  matcher_type_dist_ex,
  matcher_type_size
} matcher_type;

struct _matcher;
typedef struct _matcher *matcher_t;

struct _context;
typedef struct _context *context_t;

// base interface
//========================

matcher_t matcher_construct_by_file(matcher_type type, const char *path);
matcher_t matcher_construct_by_string(matcher_type type, const char *string);

bool matcher_destruct(matcher_t matcher);

context_t matcher_alloc_context(matcher_t matcher);
bool matcher_free_context(context_t context);

bool matcher_reset_context(context_t context, char content[], size_t len);
bool matcher_next(context_t context);

match_dict_index_ptr matcher_matched_index(context_t context);

// utils
//========================

typedef struct _idx_pos {
  const char *keyword;
  const char *extra;
  size_t length, wlen;
  size_t os, oe;
} idx_pos_s;

idx_pos_s *matcher_remaining_matched(context_t context, size_t *out_len);
idx_pos_s *matcher_match_all(context_t context, char *content, size_t len,
                             size_t *out_len);
idx_pos_s *matcher_match_with_sort(context_t context, char *content,
                                   size_t len, size_t *out_len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MATCH_MATCHER_H_ */
