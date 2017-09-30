#include "dict0.h"
#include "utf8.h"

const bool alpha_number_bitmap[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const bool alpha_bitmap[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const bool number_bitmap[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

bool dict_default_add_keyword_and_extra(match_dict_t dict,
                                        char keyword[],
                                        char extra[]);

match_dict_t dict_alloc() {
  match_dict_t p = (match_dict_t) malloc(sizeof(match_dict_s));
  if (p == NULL) return NULL;

  p->index = NULL;
  dynabuf_init(&p->buffer, 0, true);

  p->_ref_count = 1;

  p->add_keyword_and_extra = dict_default_add_keyword_and_extra;
  p->before_reset = NULL;

  return p;
}

match_dict_t dict_retain(match_dict_t dict) {
  if (dict != NULL) {
    dict->_ref_count++;
  }
  return dict;
}

void dict_release(match_dict_t dict) {
  if (dict != NULL) {
    dict->_ref_count--;
    if (dict->_ref_count == 0) {
      free(dict->index);
      dynabuf_clean(&dict->buffer);
      free(dict);
    }
  }
}

bool dict_reset(match_dict_t dict, size_t index_count, size_t buffer_size) {
  if (dict->before_reset != NULL)
    dict->before_reset(dict, &index_count, &buffer_size);

  // free memory
  free(dict->index);
  dynabuf_clean(&dict->buffer);

  dict->idx_count = 0;
  dict->idx_size = index_count + 1;
  dict->index = malloc(sizeof(match_dict_index_s) * dict->idx_size);
  if (dict->index == NULL)
    return false;

  /* 增加缓存大小, 防止溢出 */
  if (!dynabuf_init(&dict->buffer, buffer_size + 3, true)) {
    free(dict->index);
    return false;
  }

  memset(dict->index, 0, sizeof(match_dict_index_s) * dict->idx_size);
  dict->max_key_length = 0;
  dict->max_extra_length = 0;

  return true;
}

void dict_add_index(match_dict_t dict,
                    size_t length,
                    char *keyword,
                    char *extra,
                    char *tag,
                    match_dict_index_prop_f type) {
  if (dict->idx_count == dict->idx_size) {
    dict->idx_size += 100;
    dict->index = realloc(dict->index, sizeof(match_dict_s) * dict->idx_size);
    memset(dict->index + dict->idx_count, 0, sizeof(match_dict_s) * 100);
  }

  dict->index[dict->idx_count]._next = NULL;
  dict->index[dict->idx_count].length = length;
  dict->index[dict->idx_count].wlen = utf8_word_length(keyword, length);
  dict->index[dict->idx_count].keyword = keyword;
  dict->index[dict->idx_count].extra = extra;
  dict->index[dict->idx_count]._tag = tag;
  dict->index[dict->idx_count].prop = type;
  dict->idx_count++;
}

/*
 * @note
 *    虽然使用 dynabuf_s 做缓存，但缓存区不可以扩展，否则 dict 中将出现野指针
 */
bool dict_default_add_keyword_and_extra(match_dict_t dict,
                                        char keyword[],
                                        char extra[]) {
  match_dict_index_prop_f type;
  char *key_cur, *extra_cur, *tag_cur;
  size_t i, key_length;

  if (keyword != NULL && keyword[0] != '\0') {
    type = match_dict_index_prop_alnum;

    key_length = strlen(keyword);
    for (i = 0; i < key_length; i++) {
      if (!alpha_number_bitmap[(unsigned char) keyword[i]]
          && keyword[i] != ' ') {
        type = match_dict_index_prop_normal;
        break;
      }
    }

    key_cur = dynabuf_write_with_zero(&dict->buffer, keyword, key_length);

    if (key_length > dict->max_key_length)
      dict->max_key_length = key_length;

    if (extra == NULL || extra[0] == '\0') {
      extra_cur = (char *) str_empty;
      tag_cur = (char *) str_empty;
    } else {
      size_t extra_length = strlen(extra);
      char *t = strstr(extra, SEPARATOR_ID);
      if (t != NULL) {
        size_t tz = extra_length;
        extra_length = t - extra;
        extra_cur = dynabuf_write_with_zero(&dict->buffer, extra, extra_length);

        tag_cur = dynabuf_write_with_zero(&dict->buffer,
                                          t + sizeof(SEPARATOR_ID),
                                          tz - extra_length
                                              - sizeof(SEPARATOR_ID));
      } else {
        extra_cur = dynabuf_write_with_zero(&dict->buffer, extra, extra_length);
        tag_cur = (char *) str_empty;
      }

      if (extra_length > dict->max_extra_length)
        dict->max_extra_length = extra_length;
    }

    dict_add_index(dict, key_length, key_cur, extra_cur, tag_cur, type);
  }

  return true;
}

bool dict_parse(match_dict_t self, vocab_t vocab) {
  if (self == NULL || vocab == NULL) {
    return false;
  }

  size_t count = vocab_count(vocab);
  size_t length = vocab_length(vocab);
  if (!dict_reset(self, count, length + 5))
    return false;

  strlen_s keyword, extra;
  vocab_reset(vocab);
  while (vocab_next_word(vocab, &keyword, &extra)) {
    if (!self->add_keyword_and_extra(self, keyword.ptr, extra.ptr))
      return false;
  }

  return true;
}
