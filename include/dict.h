#ifndef _MATCH_DICT_H_
#define _MATCH_DICT_H_

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum match_dict_keyword_type {
  match_dict_keyword_type_normal = 0,
  match_dict_keyword_type_alpha_number,
  match_dict_keyword_type_head,
  match_dict_keyword_type_tail,
  match_dict_keyword_type_ambi,
  match_dict_keyword_type_empty,
} match_dict_keyword_type_e;

typedef struct match_dict_index {
  struct match_dict_index *_next; /* 相同前缀链表 */
  const char *keyword;
  const char *extra;
  const char *_tag;
  size_t length, wlen;
  match_dict_keyword_type_e type;
} match_dict_index_s, *match_dict_index_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MATCH_DICT_H_ */
