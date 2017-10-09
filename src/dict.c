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
                                        strlen_s keyword,
                                        strlen_s extra);

match_dict_t dict_alloc() {
  match_dict_t p = NULL;
  do {
    p = (match_dict_t) malloc(sizeof(match_dict_s));
    if (p == NULL) break;

    p->buffer = dynabuf_alloc();
    if (p->buffer == NULL) break;

    if (!dynabuf_init(p->buffer, 0, true))
      break;

    p->index = NULL;
    p->_ref_count = 1;

    p->add_keyword_and_extra = dict_default_add_keyword_and_extra;
    p->before_reset = NULL;

    return p;
  } while (0);

  if (p != NULL)
    dynabuf_free(p->buffer);

  free(p);

  return NULL;
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
      dynabuf_free(dict->buffer);
      free(dict);
    }
  }
}

bool dict_reset(match_dict_t dict, size_t index_count, size_t buffer_size) {
  if (dict->before_reset != NULL)
    dict->before_reset(dict, &index_count, &buffer_size);

  // free memory
  free(dict->index);
  dynabuf_clean(dict->buffer);

  // reset
  dict->idx_count = 0;
  dict->idx_size = index_count + 1;
  dict->index = malloc(sizeof(match_dict_index_s) * dict->idx_size);
  if (dict->index == NULL)
    return false;

  /* 增加缓存大小, 防止溢出 */
  if (!dynabuf_init(dict->buffer, buffer_size + 3, false)) {
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
                    strcur_s keyword,
                    strcur_s extra,
                    void *tag,
                    mdi_prop_f prop) {
  if (dict->idx_count == dict->idx_size) {
    dict->idx_size += 100;
    dict->index = realloc(dict->index, sizeof(match_dict_s) * dict->idx_size);
    memset(dict->index + dict->idx_count, 0, sizeof(match_dict_s) * 100);
  }

  dict->index[dict->idx_count]._next = NULL;
  dict->index[dict->idx_count].length = length;
  dict->index[dict->idx_count].wlen =
      utf8_word_length(prop & mdi_prop_bufkey
                       ? dynabuf_content(dict->buffer, keyword)
                       : keyword.ptr,
                       length);
  dict->index[dict->idx_count]._keyword = keyword;
  dict->index[dict->idx_count]._extra = extra;
  dict->index[dict->idx_count]._tag = tag;
  dict->index[dict->idx_count].prop = prop;
  dict->idx_count++;
}

bool dict_default_add_keyword_and_extra(match_dict_t dict,
                                        strlen_s keyword,
                                        strlen_s extra) {
  mdi_prop_f type = mdi_prop_alnum;
  strcur_s key_cur, extra_cur;

  if (keyword.len == 0) return true;

  for (size_t i = 0; i < keyword.len; i++) {
    if (!alpha_number_bitmap[(unsigned char) keyword.ptr[i]]
        && keyword.ptr[i] != ' ') {
      type = mdi_prop_normal;
      break;
    }
  }

  key_cur = dynabuf_write_with_zero(dict->buffer, keyword.ptr, keyword.len);

  if (keyword.len > dict->max_key_length)
    dict->max_key_length = keyword.len;

  if (extra.len == 0) {
    extra_cur = dynabuf_empty(dict->buffer);
  } else {
    extra_cur = dynabuf_write_with_zero(dict->buffer, extra.ptr, extra.len);

    if (extra.len > dict->max_extra_length)
      dict->max_extra_length = extra.len;
  }

  dict_add_index(dict, keyword.len, key_cur, extra_cur, NULL,
                 type | mdi_prop_bufkey | mdi_prop_bufextra);

  return true;
}

bool dict_parse(match_dict_t self, vocab_t vocab) {
  if (self == NULL || vocab == NULL) {
    return false;
  }

  // pre-process: set size
  size_t count = vocab_count(vocab);
  size_t length = vocab_length(vocab);
  if (!dict_reset(self, count, length + 5))
    return false;

  // parse vocab, construct dict
  strlen_s keyword, extra;
  vocab_reset(vocab);
  while (vocab_next_word(vocab, &keyword, &extra)) {
    if (!self->add_keyword_and_extra(self, keyword, extra))
      return false;
  }

  // post-process: change idx to ptr
  for (size_t i = 0; i < self->idx_count; i++) {
    match_dict_index_t idx = self->index + i;
    if (idx->prop & mdi_prop_bufkey)
      idx->_keyword = dynabuf_cur2ptr(self->buffer, idx->_keyword);
    if (idx->prop & mdi_prop_bufextra) {
      idx->_extra = dynabuf_cur2ptr(self->buffer, idx->_extra);
      if (idx->mdi_extra == NULL)
        idx->mdi_extra = str_empty;
    }
  }
  self->buffer->_prop |= dynabuf_prop_fixed;

  return true;
}
