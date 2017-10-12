#include "dict0.h"
#include "utf8.h"
#include "actrie.h"

const char tokens_delimiter = '|';
const char left_parentheses = '(';
const char right_parentheses = ')';

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

void dict_add_index_filter_free(dict_add_indix_filter filter) {
  while (filter != NULL) {
    dict_add_indix_filter next = filter->next;
    free(filter);
    filter = next;
  }
}

dict_add_indix_filter dict_add_index_filter_wrap(dict_add_indix_filter filter,
                                                 dict_add_index_func func) {
  dict_add_indix_filter node =
      malloc(sizeof(struct match_dict_add_index_filter));
  if (node == NULL) return NULL;

  node->add_index = func;
  node->next = filter;
  return node;
}

match_dict_t dict_alloc() {
  match_dict_t p = NULL;
  do {
    p = (match_dict_t) malloc(sizeof(match_dict_s));
    if (p == NULL) break;

    p->_map = NULL;
    p->buffer = dynabuf_alloc();
    if (p->buffer == NULL) break;

    if (!dynabuf_init(p->buffer, 0, true))
      break;

    p->index = NULL;
    p->_ref_count = 1;
    p->add_index_filter = NULL;
    p->before_reset = NULL;

    dict_add_indix_filter chain = p->add_index_filter;
    chain = dict_add_index_filter_wrap(chain, dict_add_index);
    chain = dict_add_index_filter_wrap(chain, dict_add_wordattr_index);
    p->add_index_filter = chain;

    return p;
  } while (0);

  if (p != NULL) {
    dynabuf_free(p->buffer);
    free(p);
  }

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
      dict_add_index_filter_free(dict->add_index_filter);
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

bool dict_add_index(match_dict_t dict, dict_add_indix_filter filter,
                    strlen_s keyword, strlen_s extra, void *tag,
                    mdi_prop_f prop) {
  strcur_s key_cur, extra_cur;

  if (dict->idx_count == dict->idx_size) {
    // extend memory
    dict->idx_size += 100;
    void *new = realloc(dict->index, sizeof(match_dict_s) * dict->idx_size);
    if (new == NULL) {
      sprintf(stderr, "%s(%d) - fatal: memory error!", __FILE__, __LINE__);
      exit(-1);
    }
    dict->index = new;
    memset(dict->index + dict->idx_count, 0, sizeof(match_dict_s) * 100);
  }

  dict->index[dict->idx_count]._next = NULL;
  dict->index[dict->idx_count].length = keyword.len;
  dict->index[dict->idx_count].wlen =
      utf8_word_length(keyword.ptr, keyword.len);

  // because we use standard c string, must use dynabuf_write_with_zero

  if (prop & mdi_prop_bufkey) {
    key_cur.ptr = trie_search(dict->_map, keyword.ptr, keyword.len);
    if (key_cur.ptr == NULL) {
      key_cur = dynabuf_write_with_zero(dict->buffer, keyword.ptr, keyword.len);
      trie_add_keyword(dict->_map, keyword.ptr, keyword.len, key_cur.ptr);
      if (keyword.len > dict->max_key_length)
        dict->max_key_length = keyword.len;
    }
  } else {
    key_cur.ptr = keyword.ptr;
  }
  dict->index[dict->idx_count]._keyword = key_cur;

  if (prop & mdi_prop_bufextra) {
    if (extra.len == 0) {
      extra_cur = dynabuf_empty_cur(dict->buffer);
    } else {
      extra_cur.ptr = trie_search(dict->_map, extra.ptr, extra.len);
      if (extra_cur.ptr == NULL) {
        extra_cur = dynabuf_write_with_zero(dict->buffer, extra.ptr, extra.len);
        trie_add_keyword(dict->_map, extra.ptr, extra.len, extra_cur.ptr);
        if (extra.len > dict->max_extra_length)
          dict->max_extra_length = extra.len;
      }
    }
  } else {
    extra_cur.ptr = extra.ptr;
  }
  dict->index[dict->idx_count]._extra = extra_cur;

  dict->index[dict->idx_count]._tag = tag;
  dict->index[dict->idx_count].prop = prop;
  dict->idx_count++;

  return true;
}

bool dict_add_wordattr_index(match_dict_t dict, dict_add_indix_filter filter,
                             strlen_s keyword, strlen_s extra, void *tag,
                             mdi_prop_f prop) {
  mdi_prop_f type = mdi_prop_alnum;

  if (keyword.len == 0) return true;

  for (size_t i = 0; i < keyword.len; i++) {
    if (!alpha_number_bitmap[(unsigned char) keyword.ptr[i]]
        && keyword.ptr[i] != ' ') {
      type = mdi_prop_word;
      break;
    }
  }

  return filter->add_index(dict, filter->next, keyword, extra, tag, type | prop);
}

bool dict_add_alternation_index(match_dict_t dict, dict_add_indix_filter filter,
                                strlen_s keyword, strlen_s extra, void *tag,
                                mdi_prop_f prop) {

  if (keyword.len == 0) return true;

  size_t so = 0, eo = keyword.len - 1;
  if ((keyword.ptr[so] == left_parentheses
      && keyword.ptr[eo] != right_parentheses)
      || (keyword.ptr[so] != left_parentheses
          && keyword.ptr[eo] == right_parentheses)) {
    filter->add_index(dict, filter->next, keyword, extra, tag, prop);
  } else {
    // 脱括号
    if (keyword.ptr[so] == left_parentheses
        && keyword.ptr[eo] == right_parentheses) {
      so++;
      eo--;
    }
    size_t i, depth = 0, cur = so;
    for (i = so; i <= eo; i++) {
      if (keyword.ptr[i] == tokens_delimiter) {
        if (depth == 0 && i > cur) {
          strlen_s key = {
              .ptr = keyword.ptr + cur,
              .len = i - cur,
          };
          filter->add_index(dict, filter->next, key, extra, tag, prop);
          cur = i + 1;
        }
      } else if (keyword.ptr[i] == left_parentheses) {
        depth++;
      } else if (keyword.ptr[i] == right_parentheses) {
        depth--;
      }
    }
    if (i > cur) {
      strlen_s key = {
          .ptr = keyword.ptr + cur,
          .len = i - cur,
      };
      filter->add_index(dict, filter->next, key, extra, tag, prop);
    }
  }

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
  self->_map = trie_alloc();
  if (self->_map == NULL) return false;
  strlen_s keyword, extra;
  vocab_reset(vocab);
  while (vocab_next_word(vocab, &keyword, &extra)) {
    // store keyword and extra in buffer
    if (!self->add_index_filter
        ->add_index(self, self->add_index_filter->next, keyword, extra, NULL,
                    mdi_prop_reserved | mdi_prop_bufkey | mdi_prop_bufextra)) {
      return false;
    }
  }
  trie_destruct(self->_map);
  self->_map = NULL;

  // post-process: change idx to ptr
  for (size_t i = 0; i < self->idx_count; i++) {
    match_dict_index_t idx = self->index + i;
    if (idx->prop & mdi_prop_bufkey)
      idx->_keyword = dynabuf_cur2ptr(self->buffer, idx->_keyword);
    if (idx->prop & mdi_prop_bufextra) {
      idx->_extra = dynabuf_cur2ptr(self->buffer, idx->_extra);
      // replace NULL with ""
      if (idx->mdi_extra == NULL) idx->mdi_extra = str_empty;
    }
    if (idx->prop & mdi_prop_tag_id)
      idx->_tag = self->index + (size_t) idx->_tag;
  }
  self->buffer->_prop |= dynabuf_prop_fixed;

  return true;
}
