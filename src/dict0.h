//
// Created by james on 9/29/17.
//

#ifndef _MATCH_DICT0_H_
#define _MATCH_DICT0_H_

#include <dict.h>
#include "dynabuf.h"
#include "vocab.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


// dictionary API
// ==============

struct match_dict;
typedef struct match_dict *match_dict_t;

struct match_dict_add_index_filter;
typedef struct match_dict_add_index_filter *dict_add_indix_filter;

typedef bool(*dict_add_index_func)
    (match_dict_t dict, dict_add_indix_filter chain, strlen_s keyword,
     strlen_s extra, void *tag, mdi_prop_f prop);
typedef void(*dict_before_reset_func)
    (match_dict_t dict, size_t *index_count, size_t *buffer_size);

struct match_dict_add_index_filter {
  dict_add_index_func add_index;
  struct match_dict_add_index_filter *next;
};

typedef struct match_dict {
  match_dict_index_t index;
  size_t idx_size, idx_count;

  dynabuf_t buffer;

  size_t max_key_length, max_extra_length;

  int _ref_count; /* 引用计数器 */

  dict_add_indix_filter add_index_filter;
  dict_before_reset_func before_reset;
} match_dict_s;

extern const bool alpha_number_bitmap[256];
extern const bool number_bitmap[256];

match_dict_t dict_alloc();
match_dict_t dict_retain(match_dict_t dict);
void dict_release(match_dict_t dict);

/**
 * if mdi_prop_bufkey is set, store keyword in buffer; else record keyword.ptr.
 * if mdi_prop_bufextra is set, store extra in buffer; else record extra.ptr.
 * @note this is filter terminal, so next will be ignored.
 */
bool dict_add_index(match_dict_t dict, dict_add_indix_filter filter,
                    strlen_s keyword, strlen_s extra, void *tag, mdi_prop_f prop);

bool dict_add_wordattr_index(match_dict_t dict, dict_add_indix_filter filter,
                             strlen_s keyword, strlen_s extra, void *tag,
                             mdi_prop_f prop);

dict_add_indix_filter dict_add_index_filter_wrap(dict_add_indix_filter filter,
                                                 dict_add_index_func func);

bool dict_parse(match_dict_t self, vocab_t vocab);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MATCH_DICT0_H_ */

