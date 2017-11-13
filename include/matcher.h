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
  matcher_type_stub = 0,     // dict store stub
  matcher_type_wordattr,     // word attribute filter
  matcher_type_alteration,   // alteration filter
  matcher_type_dat,          // double-array trie matcher
  matcher_type_acdat,        // double-array ac trie matcher
  matcher_type_seg_acdat,    // segment acdat, hang-up when fail-cursor back to root
  matcher_type_prefix_acdat, // prefix acdat, only match prefix
  matcher_type_ambi,         // ambiguous matcher
  matcher_type_dist,         // distance matcher
  matcher_type_ultimate      // combined matcher
} matcher_type_e;

struct _matcher_hdr;
typedef struct _matcher_hdr *matcher_t;

struct _context_hdr;
typedef struct _context_hdr *context_t;

// base interface
//========================

matcher_t matcher_construct_by_file(matcher_type_e type, const char *path);
matcher_t matcher_construct_by_string(matcher_type_e type, strlen_t string);

bool matcher_destruct(matcher_t matcher);

context_t matcher_alloc_context(matcher_t matcher);
bool matcher_free_context(context_t context);

bool matcher_reset_context(context_t context, char content[], size_t len);
bool matcher_next(context_t context);

mdi_t matcher_matched_index(context_t context);
strpos_s matcher_matched_pos(context_t context);
strlen_s matcher_matched_str(context_t context);

// utils
//========================

typedef struct matched_index_pos {
  mdi_t idx;
  strpos_s pos;
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
