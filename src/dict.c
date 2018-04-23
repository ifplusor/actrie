#include "dict0.h"
#include "utf8.h"
#include "actrie.h"
#include "obj/list.h"

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

match_dict_t dict_alloc() {
  match_dict_t p = NULL;
  do {
    p = (match_dict_t) amalloc(sizeof(match_dict_s));
    if (p == NULL) break;

    p->_map = NULL;

    p->index = NULL;
    p->idx_count = 0;
    p->_ref_count = 1;
    p->before_reset = NULL;

    return p;
  } while (0);

  return NULL;
}

match_dict_t dict_retain(match_dict_t dict) {
  if (dict != NULL) {
    dict->_ref_count++;
  }
  return dict;
}

bool dict_clean(match_dict_t dict) {
  if (dict == NULL) return false;
  for (size_t i = 0; i < dict->idx_count; i++) {
    mdi_t idx = &dict->index[i];
    if (idx->prop & mdi_prop_bufkey) {
      _release(cstr2dstr(idx->keyword));
    }
    if (idx->prop & mdi_prop_bufextra) {
      if (idx->extra != str_empty)
        _release(cstr2dstr(idx->extra));
    }
  }
  free(dict->index);
  dict->index = NULL;

  return true;
}

void dict_release(match_dict_t dict) {
  if (dict != NULL) {
    dict->_ref_count--;
    if (dict->_ref_count == 0) {
      dict_clean(dict);
      afree(dict);
    }
  }
}

bool dict_reset(match_dict_t dict, size_t index_count, size_t buffer_size) {
  if (dict->before_reset != NULL)
    dict->before_reset(dict, &index_count, &buffer_size);

  // free memory
  dict_clean(dict);

  // reset
  dict->idx_count = 0;
  dict->idx_size = index_count + 1;
  dict->index = malloc(sizeof(mdi_s) * dict->idx_size);
  if (dict->index == NULL)
    return false;

  memset(dict->index, 0, sizeof(mdi_s) * dict->idx_size);
  dict->max_key_length = 0;
  dict->max_extra_length = 0;

  return true;
}

bool dict_add_index(match_dict_t dict, matcher_conf_t conf, strlen_s keyword,
                    strlen_s extra, void *tag, mdi_prop_f prop) {

  if (keyword.len == 0) return true;

  if (dict->idx_count == dict->idx_size) {
    // extend memory
    dict->idx_size += 100;
    void *new = realloc(dict->index, sizeof(match_dict_s) * dict->idx_size);
    if (new == NULL) {
      fprintf(stderr, "%s(%d) - fatal: memory error!", __FILE__, __LINE__);
      exit(-1);
    }
    dict->index = new;
    memset(dict->index + dict->idx_count, 0, sizeof(match_dict_s) * 100);
  }

  aobj ds = NULL;
  if (prop & mdi_prop_bufkey) {
    aobj list = trie_search(dict->_map, keyword.ptr, keyword.len);
    if (list == NULL) {
      ds = dstr_with_buf(keyword.ptr, keyword.len);
      // store suffix
      trie_add_keyword(dict->_map, keyword.ptr, keyword.len, ds);
      if (keyword.len > dict->max_key_length)
        dict->max_key_length = keyword.len;
    } else {
      ds = _(list, list, car);
      _retain(ds);
    }
  } else {
    ds = keyword.ptr;
  }
  dict->index[dict->idx_count].keyword = ds; // key_cur;

  if (prop & mdi_prop_bufextra) {
    if (extra.len == 0) {
      ds = NULL;
    } else {
      aobj list = trie_search(dict->_map, extra.ptr, extra.len);
      if (list == NULL) {
        ds = dstr_with_buf(extra.ptr, extra.len);
        // store suffix
        trie_add_keyword(dict->_map, extra.ptr, extra.len, ds);
        if (extra.len > dict->max_extra_length)
          dict->max_extra_length = extra.len;
      } else {
        ds = _(list, list, car);
        _retain(ds);
      }
    }
  } else {
    ds = extra.ptr;
  }
  dict->index[dict->idx_count].extra = ds;

  dict->index[dict->idx_count]._tag = tag;
  dict->index[dict->idx_count].prop = mdi_prop_set_matcher(prop, conf->id);
  dict->idx_count++;

  return true;
}

bool dict_add_wordattr_index(match_dict_t dict, matcher_conf_t conf,
                             strlen_s keyword, strlen_s extra, void *tag,
                             mdi_prop_f prop) {
  matcher_conf_t stub_conf = ((stub_conf_t) conf->buf)->stub;
  uint32_t type = mdi_prop_alnum;

  if (keyword.len == 0) return true;

  for (size_t i = 0; i < keyword.len; i++) {
    if (!alpha_number_bitmap[(unsigned char) keyword.ptr[i]]
        && keyword.ptr[i] != ' ') {
      type = mdi_prop_word;
      break;
    }
  }

  return stub_conf->add_index(dict, stub_conf, keyword, extra, tag, type | prop);
}

bool dict_add_alternation_index(match_dict_t dict, matcher_conf_t conf,
                                strlen_s keyword, strlen_s extra, void *tag,
                                mdi_prop_f prop) {
  matcher_conf_t stub_conf = ((stub_conf_t) conf->buf)->stub;

  if (keyword.len == 0) return true;

  size_t so = 0, eo = keyword.len - 1;
  size_t i, depth = 0;

  /* 脱括号 */
  while (keyword.ptr[so] == T_PACKET_L && keyword.ptr[eo] == T_PACKET_R) {
    for (i = so + 1; i <= eo - 1; i++) {
      if (keyword.ptr[i] == T_PACKET_L) {
        depth++;
      } else if (keyword.ptr[i] == T_PACKET_R) {
        if (depth <= 0) break; // error
        depth--;
      }
    }
    if (i < eo) break;
    so++;
    eo--;
  }

  depth = 0;
  size_t cur = so;
  for (i = so; i <= eo; i++) {
    if (keyword.ptr[i] == T_ALTER) {
      if (depth == 0 && i > cur) {
        strlen_s key = {
            .ptr = keyword.ptr + cur,
            .len = i - cur,
        };
        stub_conf->add_index(dict, stub_conf, key, extra, tag, prop);
        cur = i + 1;
      }
    } else if (keyword.ptr[i] == T_PACKET_L) {
      depth++;
    } else if (keyword.ptr[i] == T_PACKET_R) {
      depth--;
    }
  }
  if (i > cur) {
    strlen_s key = {
        .ptr = keyword.ptr + cur,
        .len = i - cur,
    };
    stub_conf->add_index(dict, stub_conf, key, extra, tag, prop);
  }

  return true;
}

bool is_dist(const char *ptr, size_t len) {
  size_t i = 0;
  if (i >= len || ptr[i++] != '{') return false;
  if (i >= len || ptr[i] < '0' || ptr[i] > '9') return false;
  do { i++; } while (i < len && ptr[i] >= '0' && ptr[i] < '9');
  if (i >= len || ptr[i++] != ',') return false;
  if (i >= len || ptr[i] < '0' || ptr[i] > '9') return false;
  do { i++; } while (i < len && ptr[i] >= '0' && ptr[i] < '9');
  if (i >= len || ptr[i++] != '}') return false;
  return i < len;
}

strlen_s transform_keyword(strlen_t keyword) {
  char *new_keyword = malloc(keyword->len + 1);
  if (new_keyword == NULL) {
    return (strlen_s) {.ptr=NULL, .len=0};
  }

  size_t cur = 0;
  bool is_escape = false;
  for (int i = 0; i < keyword->len; i++) {
    char ch = keyword->ptr[i];
    if (is_escape) {
      if ('d' == ch && is_dist(keyword->ptr + i + 1, keyword->len - i - 1)) {
        new_keyword[cur++] = T_NUM;
        i += 2;
      } else if ('n' == ch) {
        new_keyword[cur++] = '\n';
      } else if ('r' == ch) {
        new_keyword[cur++] = '\r';
      } else if ('t' == ch) {
        new_keyword[cur++] = '\t';
      } else {
        new_keyword[cur++] = ch;
      }
      is_escape = false;
      continue;
    }
    if ('\\' == ch) {
      is_escape = true;
      continue;
    } else if ('|' == ch) {
      new_keyword[cur++] = T_ALTER;
    } else if ('(' == ch) {
      new_keyword[cur++] = T_PACKET_L;
    } else if (')' == ch) {
      new_keyword[cur++] = T_PACKET_R;
    } else if ('.' == ch && is_dist(keyword->ptr + i + 1, keyword->len - i - 1)) {
      new_keyword[cur++] = T_ANY;
    } else if ('?' == ch && keyword->len - i > 3 && '&' == keyword->ptr[i + 1] && '!' == keyword->ptr[i + 2]) {
      new_keyword[cur++] = T_AMBI;
      i += 2;
    } else {
      new_keyword[cur++] = ch;
    }
  }
  new_keyword[cur] = '\0';
  return (strlen_s) {.ptr=new_keyword, .len=cur};
}

bool dict_parse(match_dict_t self, vocab_t vocab, matcher_conf_t conf) {
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
    // TODO: check syntax

    keyword = transform_keyword(&keyword);

    // store keyword and extra in buffer
    if (!conf->add_index(self, conf, keyword, extra, NULL,
                         mdi_prop_reserved | mdi_prop_bufkey | mdi_prop_bufextra)) {
      free(keyword.ptr);
      return false;
    }
    free(keyword.ptr);
  }
  trie_destruct(self->_map);
  self->_map = NULL;

  // post-process: change dstr to cstr
  for (size_t i = 0; i < self->idx_count; i++) {
    mdi_t idx = self->index + i;
    if (idx->prop & mdi_prop_bufkey)
      idx->keyword = dstr2cstr((dstr_t) idx->keyword);
    if (idx->prop & mdi_prop_bufextra) {
      idx->extra = dstr2cstr((dstr_t) idx->extra);
      // replace NULL with ""
      if (idx->extra == NULL) idx->extra = str_empty;
    }
    if (idx->prop & mdi_prop_tag_id)
      idx->_tag = self->index + (size_t) idx->_tag;
  }

  return true;
}
