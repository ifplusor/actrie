#ifndef _MATCH_DICT_H_
#define _MATCH_DICT_H_

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SEPARATOR_ID "###"

typedef enum match_dict_keyword_type {
  match_dict_keyword_type_normal = 0,
  match_dict_keyword_type_alpha_number,
  match_dict_keyword_type_head,
  match_dict_keyword_type_tail,
  match_dict_keyword_type_empty
} match_dict_keyword_type;

typedef struct match_dict_index {
  struct match_dict_index *next; /* 相同前缀链表 */
  const char *keyword;
  const char *extra;
  const char *tag;
  size_t length;
  match_dict_keyword_type type;
} match_dict_index, *match_dict_index_ptr;

typedef struct match_dict match_dict, *match_dict_ptr;

typedef bool(*dict_add_keyword_and_extra)
    (match_dict_ptr dict, char keyword[], char extra[]);
typedef void(*dict_before_reset)
    (match_dict_ptr dict, size_t *index_count, size_t *buffer_size);

struct match_dict {
  match_dict_index_ptr index;
  size_t idx_size, idx_count;

  char *buffer;
  size_t buf_size;

  size_t max_key_length, max_extra_length;

  size_t _cursor;
  int _ref_count; /* 引用计数器 */

  dict_add_keyword_and_extra add_keyword_and_extra;
  dict_before_reset before_reset;
};

extern const bool alpha_number_bitmap[256];

match_dict_ptr dict_alloc();
match_dict_ptr dict_assign(match_dict_ptr dict);
void dict_release(match_dict_ptr dict);

void dict_add_index(match_dict_ptr dict,
                    size_t length,
                    char *keyword,
                    char *extra,
                    char *tag,
                    match_dict_keyword_type type);

bool dict_parser_by_file(match_dict_ptr dict, FILE *fp);
bool dict_parser_by_s(match_dict_ptr dict, const char *s);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MATCH_DICT_H_ */
