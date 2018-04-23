//
// Created by james on 9/29/17.
//

#ifndef _ACTRIE_DICT0_H_
#define _ACTRIE_DICT0_H_

#include <dict.h>
#include "configure.h"
#include "dynabuf.h"
#include "vocab.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define T_PACKET_L 0x01
#define T_PACKET_R 0x02
#define T_ALTER 0x03
#define T_ANY 0x04
#define T_NUM 0x05
#define T_AMBI 0x06


// dictionary API
// ==============

typedef void(*dict_before_reset_func)
    (match_dict_t dict, size_t *index_count, size_t *buffer_size);

typedef struct match_dict {
  mdi_t index;
  size_t idx_size, idx_count;

  void *_map;       /* reserved, duplicate checking for buffer */

  size_t max_key_length, max_extra_length;

  int _ref_count; /* 引用计数器 */

  dict_before_reset_func before_reset;
} match_dict_s;

match_dict_t dict_alloc();
match_dict_t dict_retain(match_dict_t dict);
void dict_release(match_dict_t dict);

/**
 * if mdi_prop_bufkey is set, store keyword in buffer; else record keyword.ptr.
 * if mdi_prop_bufextra is set, store extra in buffer; else record extra.ptr.
 * @note this is filter terminal, so next will be ignored.
 */
bool dict_add_index(match_dict_t dict, matcher_conf_t conf, strlen_s keyword,
                    strlen_s extra, void *tag, mdi_prop_f prop);

bool dict_add_wordattr_index(match_dict_t dict, matcher_conf_t conf,
                             strlen_s keyword, strlen_s extra, void *tag,
                             mdi_prop_f prop);

bool dict_add_alternation_index(match_dict_t dict, matcher_conf_t conf,
                                strlen_s keyword, strlen_s extra, void *tag,
                                mdi_prop_f prop);

bool dict_parse(match_dict_t self, vocab_t vocab, matcher_conf_t conf);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ACTRIE_DICT0_H_ */

