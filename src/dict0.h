//
// Created by james on 9/29/17.
//

#ifndef _MATCH_DICT0_H_
#define _MATCH_DICT0_H_

#include <dict.h>
#include "dynabuf.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


// dictionary API
// ==============

#define SEPARATOR_ID "###"

struct match_dict;
typedef struct match_dict match_dict_s, *match_dict_t;

typedef bool(*dict_add_keyword_and_extra)
    (match_dict_t dict, char keyword[], char extra[]);
typedef void(*dict_before_reset)
    (match_dict_t dict, size_t *index_count, size_t *buffer_size);

struct match_dict {
  match_dict_index_t index;
  size_t idx_size, idx_count;

  dynabuf_s buffer;
  char *empty;

  size_t max_key_length, max_extra_length;

  int _ref_count; /* 引用计数器 */

  dict_add_keyword_and_extra add_keyword_and_extra;
  dict_before_reset before_reset;
};

extern const bool alpha_number_bitmap[256];
extern const bool number_bitmap[256];

match_dict_t dict_alloc();
match_dict_t dict_assign(match_dict_t dict);
void dict_release(match_dict_t dict);

void dict_add_index(match_dict_t dict,
                    size_t length,
                    char *keyword,
                    char *extra,
                    char *tag,
                    match_dict_keyword_type_e type);

bool dict_parser_by_file(match_dict_t dict, FILE *fp);
bool dict_parser_by_s(match_dict_t dict, const char *s);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MATCH_DICT0_H_ */

