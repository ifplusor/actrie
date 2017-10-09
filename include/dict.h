#ifndef _MATCH_DICT_H_
#define _MATCH_DICT_H_

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef enum match_dict_index_prop {
  mdi_prop_empty = 0,
  mdi_prop_normal = 1,
  mdi_prop_ambi = 2,
  mdi_prop_alnum = 4,
  mdi_prop_head = 8,
  mdi_prop_tail = 16,
  mdi_prop_bufkey = 32,
  mdi_prop_bufextra = 64,
} mdi_prop_f;

typedef struct match_dict_index {
  struct match_dict_index *_next; /* 相同前缀链表 */
  strcur_s _keyword;
#define mdi_keyword _keyword.ptr
  strcur_s _extra;
#define mdi_extra _extra.ptr
  void *_tag;
  size_t length, wlen;
  mdi_prop_f prop;
} match_dict_index_s, *match_dict_index_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MATCH_DICT_H_ */
