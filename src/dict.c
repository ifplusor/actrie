#include "dict0.h"
#include "utf8.h"
#include "actrie.h"
#include "list.h"

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

match_dict_t dict_alloc() {
  match_dict_t p = NULL;
  do {
    p = (match_dict_t) amalloc(sizeof(match_dict_s));
    if (p == NULL) break;

    p->_map = NULL;

    p->index = NULL;
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
      _release(cstr2dstr(idx->_keyword));
    }
    if (idx->prop & mdi_prop_bufextra) {
      _release(cstr2dstr(idx->_extra));
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

bool dict_add_index(match_dict_t dict, matcher_config_t config,
                    strlen_s keyword, strlen_s extra, void *tag,
                    mdi_prop_f prop) {

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

  // NOTE: mdi.length is uint16_t
  dict->index[dict->idx_count].length = (uint16_t) keyword.len;
  dict->index[dict->idx_count].wlen = (uint16_t)
      utf8_word_length(keyword.ptr, keyword.len);

  // because we use standard c string, must use dynabuf_write_with_zero

  aobj ds = NULL;
  if (prop & mdi_prop_bufkey) {
    aobj list = trie_search(dict->_map, keyword.ptr, keyword.len);
    if (list == NULL) {
      ds = dstr(keyword.ptr, keyword.len);
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
  dict->index[dict->idx_count]._keyword = ds; // key_cur;

  if (prop & mdi_prop_bufextra) {
    if (extra.len == 0) {
      ds = NULL;
    } else {
      aobj list = trie_search(dict->_map, extra.ptr, extra.len);
      if (list == NULL) {
        ds = dstr(extra.ptr, extra.len);
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
  dict->index[dict->idx_count]._extra = ds;

  dict->index[dict->idx_count]._tag = tag;
  dict->index[dict->idx_count].prop = mdi_prop_set_matcher(prop, config->id);
  dict->idx_count++;

  return true;
}

bool dict_add_wordattr_index(match_dict_t dict, matcher_config_t config,
                             strlen_s keyword, strlen_s extra, void *tag,
                             mdi_prop_f prop) {
  matcher_config_t stub_config = config->config;
  uint32_t type = mdi_prop_alnum;

  if (keyword.len == 0) return true;

  for (size_t i = 0; i < keyword.len; i++) {
    if (!alpha_number_bitmap[(unsigned char) keyword.ptr[i]]
        && keyword.ptr[i] != ' ') {
      type = mdi_prop_word;
      break;
    }
  }

  return stub_config->add_index(dict, stub_config, keyword, extra, tag,
                                type | prop);
}

bool dict_add_alternation_index(match_dict_t dict, matcher_config_t config,
                                strlen_s keyword, strlen_s extra, void *tag,
                                mdi_prop_f prop) {
  matcher_config_t stub_config = config->config;
  if (keyword.len == 0) return true;

  size_t so = 0, eo = keyword.len - 1;
  if ((keyword.ptr[so] == left_parentheses
      && keyword.ptr[eo] != right_parentheses)
      || (keyword.ptr[so] != left_parentheses
          && keyword.ptr[eo] == right_parentheses)) {
    stub_config->add_index(dict, stub_config, keyword, extra, tag, prop);
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
          stub_config->add_index(dict, stub_config, key, extra, tag, prop);
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
      stub_config->add_index(dict, stub_config, key, extra, tag, prop);
    }
  }

  return true;
}

bool dict_parse(match_dict_t self, vocab_t vocab, matcher_config_t conf) {
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
    if (!conf
        ->add_index(self, conf, keyword, extra, NULL,
                    mdi_prop_reserved | mdi_prop_bufkey | mdi_prop_bufextra)) {
      return false;
    }
  }
  trie_destruct(self->_map);
  self->_map = NULL;

  // post-process: change dstr to cstr
  for (size_t i = 0; i < self->idx_count; i++) {
    mdi_t idx = self->index + i;
    if (idx->prop & mdi_prop_bufkey)
      idx->_keyword = dstr2cstr(idx->_keyword);
    if (idx->prop & mdi_prop_bufextra) {
      idx->_extra = dstr2cstr(idx->_extra);
      // replace NULL with ""
      if (idx->mdi_extra == NULL) idx->mdi_extra = str_empty;
    }
    if (idx->prop & mdi_prop_tag_id)
      idx->_tag = self->index + (size_t) idx->_tag;
  }

  return true;
}
