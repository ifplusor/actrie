//
// Created by james on 6/16/17.
//

#include "matcher0.h"
#include "vocab.h"
#include "acdat.h"
#include "disambi.h"
#include "distance.h"

matcher_t matcher_construct_by_dict(match_dict_t dict, matcher_config_t conf) {
  matcher_t matcher = NULL;
  switch (conf->type) {
    case matcher_type_alteration:
      matcher = matcher_construct_by_dict(dict, conf->config);
      break;
    case matcher_type_dat:
    case matcher_type_acdat:
    case matcher_type_seg_acdat:
    case matcher_type_prefix_acdat:
      matcher = dat_construct(dict, conf);
      break;
    case matcher_type_ambi:
      matcher = ambi_construct(dict, conf);
      break;
    case matcher_type_dist:
    case matcher_type_ultimate:
      matcher = dist_construct(dict, conf);
      break;
    default:
      break;
  }
  return matcher;
}

matcher_t matcher_construct_by_vocab(vocab_t vocab, matcher_config_t conf) {
  match_dict_t dict = NULL;
  matcher_t matcher = NULL;

  do {
    dict = dict_alloc();
    if (dict == NULL) break;

    if (!dict_parse(dict, vocab, conf)) break;

    matcher = matcher_construct_by_dict(dict, conf);
  } while (0);

  dict_release(dict);

  return matcher;
}

matcher_config_t matcher_ultimate_config() {
  matcher_config_t dist_config = NULL;
  matcher_config_t head_config = NULL;
  matcher_config_t tail_config = NULL;
  matcher_config_t digit_config = NULL;
  uint8_t matcher_id = 0;

  matcher_id++;
  head_config = matcher_stub_config(matcher_id, NULL);
  head_config = matcher_wordattr_config(matcher_id, head_config);
  head_config = dat_matcher_config(matcher_id, true, head_config);
  head_config->type = matcher_type_seg_acdat;
  head_config = matcher_alternation_config(matcher_id, head_config);

  matcher_id++;
  head_config = ambi_matcher_config(matcher_id, head_config);
  head_config = matcher_alternation_config(matcher_id, head_config);

  matcher_id++;
  tail_config = matcher_stub_config(matcher_id, NULL);
  tail_config = matcher_wordattr_config(matcher_id, tail_config);
  tail_config = dat_matcher_config(matcher_id, true, tail_config);
  tail_config->type = matcher_type_seg_acdat;
  tail_config = matcher_alternation_config(matcher_id, tail_config);

  matcher_id++;
  tail_config = ambi_matcher_config(matcher_id, tail_config);
  tail_config = matcher_alternation_config(matcher_id, tail_config);

  matcher_id++;
  digit_config = matcher_stub_config(matcher_id, NULL);
  digit_config = matcher_wordattr_config(matcher_id, digit_config);
  digit_config = dat_matcher_config(matcher_id, true, digit_config);
  digit_config->type = matcher_type_prefix_acdat;
  digit_config = matcher_alternation_config(matcher_id, digit_config);

  matcher_id++;
  dist_config =
      dist_matcher_config(matcher_id, head_config, tail_config, digit_config);

  return dist_config;
}

matcher_t matcher_construct(matcher_type_e type, vocab_t vocab) {
  matcher_t matcher = NULL;
  matcher_config_t config = NULL;

  do {
    if (vocab == NULL) break;

    switch (type) {
      case matcher_type_ultimate:
        config = matcher_ultimate_config();
        break;
      default:break;
    }

    if (config == NULL) break;

    matcher = matcher_construct_by_vocab(vocab, config);
  } while (0);

  return matcher;
}

matcher_t matcher_construct_by_file(matcher_type_e type, const char *path) {
  vocab_t vocab = vocab_construct(stream_type_file, path);
  matcher_t matcher = matcher_construct(type, vocab);
  vocab_destruct(vocab);
  return matcher;
}

matcher_t matcher_construct_by_string(matcher_type_e type, const char *string) {
  vocab_t vocab = vocab_construct(stream_type_string, string);
  matcher_t matcher = matcher_construct(type, vocab);
  vocab_destruct(vocab);
  return matcher;
}

bool matcher_destruct(matcher_t matcher) {
  if (matcher == NULL) return false;
  return matcher->_func.destruct(matcher);
}

context_t matcher_alloc_context(matcher_t matcher) {
  if (matcher == NULL) return NULL;
  return matcher->_func.alloc_context(matcher);
}

bool matcher_free_context(context_t context) {
  if (context == NULL) return false;
  return context->_func.free_context(context);
}

bool matcher_reset_context(context_t context, char content[], size_t len) {
  if (context == NULL) return false;
  return context->_func.reset_context(context, content, len);
}

bool matcher_next(context_t context) {
  if (context == NULL) return false;
  return context->_func.next(context);
}

inline mdi_t matcher_matched_index(context_t context) {
  return context->out_matched_index;
}

inline strpos_s matcher_matched_pos(context_t context) {
  return (strpos_s) {
      .so = context->out_eo - context->out_matched_index->length,
      .eo = context->out_eo
  };
}


#ifndef MATCHER_ALLOC_MIN_SIZE
#define MATCHER_ALLOC_MIN_SIZE 16
#endif

/**
 * @note need call free() for return value
 */
idx_pos_s *matcher_remaining_matched(context_t context, size_t *out_len) {
  size_t size = MATCHER_ALLOC_MIN_SIZE, len = 0;
  idx_pos_s *lst = malloc(sizeof(idx_pos_s) * size);
  if (lst != NULL) {
    while (matcher_next(context)) {
      if (len == size) {
        size <<= 1;
        idx_pos_s *new = realloc(lst, sizeof(idx_pos_s) * size);
        if (new == NULL) break; else lst = new;
      }
      mdi_t p = context->out_matched_index;
      lst[len].keyword = p->mdi_keyword;
      lst[len].extra = p->mdi_extra;
      lst[len].length = p->length;
      lst[len].wlen = p->wlen;
      lst[len].eo = context->out_eo;
      lst[len].so = context->out_eo - p->length;
      len++;
    }
  }
  *out_len = len;
  return lst;
}

idx_pos_s *matcher_match_all(context_t context, char *content, size_t len,
                             size_t *out_len) {
  matcher_reset_context(context, content, len);
  return matcher_remaining_matched(context, out_len);
}

int cmp(const void *a, const void *b) {
  idx_pos_s *sa = a, *sb = b;
  if (sa->so == sb->so) {
    return (int)sa->eo - (int)sb->eo;
  } else {
    return (int)sa->so - (int)sb->so;
  }
}

idx_pos_s *matcher_sort_by_os(idx_pos_s *lst, size_t len) {
  if (lst != NULL && len != 0)
    qsort(lst, len, sizeof(idx_pos_s), cmp);
  return lst;
}

inline idx_pos_s *matcher_match_with_sort(context_t context, char *content,
                                          size_t len, size_t *out_len) {
  idx_pos_s *lst = matcher_match_all(context, content, len, out_len);
  return matcher_sort_by_os(lst, *out_len);
}
