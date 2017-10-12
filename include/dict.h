#ifndef _MATCH_DICT_H_
#define _MATCH_DICT_H_

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef enum match_dict_index_prop {
  mdi_prop_empty = 0,
  mdi_prop_reserved = 1,

  // store attr
  mdi_prop_bufkey = 2,
  mdi_prop_bufextra = 4,

  // word attr
  mdi_prop_word = 8,
  mdi_prop_alnum = 16,

  // ambi attr
  mdi_prop_normal = 32,
  mdi_prop_ambi = 64,

  // dist attr
  mdi_prop_single = 128,
  mdi_prop_head = 256,
  mdi_prop_tail = 512,
  mdi_prop_dist_digit = 1024,

  // post process attr
  mdi_prop_tag_id = 2048,

} mdi_prop_f;

typedef struct match_dict_index {
  struct match_dict_index *_next; /* 相同前缀链表 */

  strcur_s _keyword;            /* 匹配的正文文本 */
#define mdi_keyword _keyword.ptr
  strcur_s _extra;
#define mdi_extra _extra.ptr
  void *_tag;
  size_t length, wlen;    /* 文本的字节、字符长度 */
  mdi_prop_f prop;
} match_dict_index_s, *match_dict_index_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MATCH_DICT_H_ */
