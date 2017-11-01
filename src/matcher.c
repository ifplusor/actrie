//
// Created by james on 6/16/17.
//

#include "matcher0.h"
#include "vocab.h"
#include "acdat.h"
#include "disambi.h"
#include "distance.h"

matcher_t matcher_construct_by_dict(match_dict_t dict, aobj conf) {
  matcher_config_t config = GET_AOBJECT(conf);
  matcher_t matcher = NULL;
  switch (config->type) {
    case matcher_type_alteration:
      matcher = matcher_construct_by_dict(dict, ((stub_config_t)config->buf)->stub);
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

matcher_t matcher_construct_by_vocab(vocab_t vocab, aobj conf) {
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

aobj matcher_ultimate_conf() {
  aobj dist_conf = NULL;
  aobj head_conf = NULL;
  aobj tail_conf = NULL;
  aobj digit_conf = NULL;
  uint8_t matcher_id = 0;

  matcher_id++;
  head_conf = matcher_root_conf(matcher_id);
  head_conf = matcher_wordattr_conf(matcher_id, head_conf);
  head_conf = dat_matcher_conf(matcher_id, matcher_type_seg_acdat, head_conf);
  head_conf = matcher_alternation_conf(matcher_id, head_conf);

  matcher_id++;
  head_conf = ambi_matcher_conf(matcher_id, head_conf);
  head_conf = matcher_alternation_conf(matcher_id, head_conf);

  matcher_id++;
  tail_conf = matcher_root_conf(matcher_id);
  tail_conf = matcher_wordattr_conf(matcher_id, tail_conf);
  tail_conf = dat_matcher_conf(matcher_id, matcher_type_seg_acdat, tail_conf);
  tail_conf = matcher_alternation_conf(matcher_id, tail_conf);

  matcher_id++;
  tail_conf = ambi_matcher_conf(matcher_id, tail_conf);
  tail_conf = matcher_alternation_conf(matcher_id, tail_conf);

  matcher_id++;
  digit_conf = matcher_root_conf(matcher_id);
  digit_conf = matcher_wordattr_conf(matcher_id, digit_conf);
  digit_conf = dat_matcher_conf(matcher_id, matcher_type_prefix_acdat, digit_conf);
  digit_conf = matcher_alternation_conf(matcher_id, digit_conf);

  matcher_id++;
  dist_conf = dist_matcher_conf(matcher_id, head_conf, tail_conf, digit_conf);
  dist_conf = matcher_alternation_conf(matcher_id, dist_conf);

  return dist_conf;
}

matcher_t matcher_construct(matcher_type_e type, vocab_t vocab) {
  matcher_t matcher = NULL;
  aobj conf = NULL;

  do {
    if (vocab == NULL) break;

    switch (type) {
      case matcher_type_ultimate:
        conf = matcher_ultimate_conf();
        break;
      default:break;
    }

    if (conf == NULL) break;

    matcher = matcher_construct_by_vocab(vocab, conf);
    _release(conf);
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

mdi_t matcher_matched_index(context_t context) {
  return context->out_matched_index;
}

strpos_s matcher_matched_pos(context_t context) {
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
