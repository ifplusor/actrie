//
// Created by james on 6/16/17.
//

#include "matcher0.h"
#include "vocab.h"
#include "acdat.h"
#include "disambi.h"
#include "distance.h"

const matcher_func_t const matcher_func_table[matcher_type_size] = {
    [matcher_type_dat] = &dat_matcher_func,
    [matcher_type_acdat] = &dat_matcher_func,
    [matcher_type_distance] = &dist_matcher_func,
};

const context_func_t const context_func_table[matcher_type_size] = {
    [matcher_type_dat] = &dat_context_func,
    [matcher_type_acdat] = &acdat_context_func,
    [matcher_type_distance] = &dist_context_func,
};

matcher_t matcher_construct(matcher_type_e type, vocab_t vocab) {
  matcher_t matcher = NULL;
  if (vocab == NULL) return NULL;

  switch (type) {
    case matcher_type_dat:
      matcher = (matcher_t) dat_construct(vocab, false);
      break;
    case matcher_type_acdat:
      matcher = (matcher_t) dat_construct(vocab, true);
      break;
    case matcher_type_ambi:
//      matcher = (matcher_t) ambi_construct(vocab, 0, true);
      break;
    case matcher_type_distance:
      matcher = (matcher_t) dist_construct(vocab, true);
      break;
    default:break;
  }

  if (matcher != NULL) {
    matcher->_type = type;
    matcher->_func = *matcher_func_table[type];
  }

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
  if (matcher == NULL) {
    return false;
  }
  return matcher->_func.destruct(matcher);
}

context_t matcher_alloc_context(matcher_t matcher) {
  context_t context = NULL;
  if (matcher == NULL) {
    return NULL;
  }
  context = matcher->_func.alloc_context(matcher);
  if (context != NULL) {
    context->_type = matcher->_type;
    context->_func = context_func_table[context->_type];
  }
  return context;
}

bool matcher_free_context(context_t context) {
  if (context == NULL) {
    return false;
  }
  return context->_func->free_context(context);
}

bool matcher_reset_context(context_t context, char content[], size_t len) {
  if (context == NULL) {
    return false;
  }
  return context->_func->reset_context(context, content, len);
}

bool matcher_next(context_t context) {
  if (context == NULL) {
    return false;
  }
  return context->_func->next(context);
}

inline match_dict_index_t matcher_matched_index(context_t context) {
  return context->out_matched_index;
}

inline strpos_s matcher_matched_pos(context_t context) {
  return {
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
      match_dict_index_t p = context->out_matched_index;
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