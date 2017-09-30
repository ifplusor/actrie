#ifndef _MATCH_DICT_H_
#define _MATCH_DICT_H_

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum match_dict_index_prop {
  match_dict_index_prop_empty = 0,
  match_dict_index_prop_normal = 1,
  match_dict_index_prop_ambi = 2,
  match_dict_index_prop_alnum = 4,
  match_dict_index_prop_head = 8,
  match_dict_index_prop_tail = 16,
} match_dict_index_prop_f;

typedef struct match_dict_index {
  struct match_dict_index *_next; /* 相同前缀链表 */
  const char *keyword;
  const char *extra;
  const char *_tag;
  size_t length, wlen;
  match_dict_index_prop_f prop;
} match_dict_index_s, *match_dict_index_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MATCH_DICT_H_ */
