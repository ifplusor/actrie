//
// Created by james on 9/27/17.
//

#include <regex.h>
#include "disambi.h"

const matcher_func_l ambi_matcher_func = {
    .destruct = (matcher_destruct_func) ambi_destruct,
    .alloc_context = (matcher_alloc_context_func) ambi_alloc_context
};

const context_func_l ambi_context_func = {
    .free_context = (matcher_free_context_func) ambi_free_context,
    .reset_context = (matcher_reset_context_func) ambi_reset_context,
    .next = (matcher_next_func) ambi_next_on_index
};

/*
 * pattern will match:
 * - A(?&!D1|D2)
 */
static const char *pattern = "([^()]*?)(\\(\\?&!(.*?)\\))?";
static regex_t reg;
static bool pattern_compiled = false;

bool ambi_dict_add_index(match_dict_t dict,
                         dict_add_indix_filter filter,
                         strlen_s keyword,
                         strlen_s extra,
                         void * tag,
                         mdi_prop_f prop) {
  int err;

  if (!pattern_compiled) {
    // compile pattern
    err = regcomp(&reg, pattern, REG_EXTENDED | REG_NEWLINE);
    if (err) {
      fprintf(stderr, "%s(%d) - error: compile regex failed!\n",
              __FILE__, __LINE__);
      exit(-1);
    } else {
      pattern_compiled = true;
    }
  }

  if (keyword.len == 0) return true;

  regmatch_t pmatch[4];
  err = regexec(&reg, keyword.ptr, 4, pmatch, 0);
  if (err == REG_NOMATCH) {
    // non-ambi
    filter->add_index(dict, filter->next, keyword, extra, tag,
                      mdi_prop_normal | prop);
    return true;
  } else if (err != REG_NOERROR) {
    return false;
  }

  mdi_prop_f base_prop = mdi_prop_tag_id | mdi_prop_bufkey;

  // store original keyword
  void *key_tag = (void *) dict->idx_count;
  dict_add_index(dict, NULL, keyword, extra, tag, prop);

  // store processed keyword
  strlen_s key = {
      .ptr = keyword.ptr + pmatch[1].rm_so,
      .len = (size_t) (pmatch[1].rm_eo - pmatch[1].rm_so),
  };

  strlen_s ambi = {
      .ptr = keyword.ptr + pmatch[3].rm_so,
      .len = (size_t) (pmatch[3].rm_eo - pmatch[3].rm_so),
  };

  filter->add_index(dict, filter->next, key, strlen_empty, key_tag,
                    mdi_prop_normal | base_prop);

  filter->add_index(dict, filter->next, ambi, strlen_empty, key_tag,
                    mdi_prop_ambi | base_prop);

  return true;
}

mdi_prop_f base_flag = mdi_prop_ambi;

ambi_matcher_t ambi_construct_by_dict(match_dict_t dict,
                                      mdi_prop_f filter,
                                      bool enable_automation) {
  ambi_matcher_t matcher = NULL;
  trie_t trie = NULL;

  do {
    trie = trie_construct_by_dict(dict, base_flag | filter, false);
    if (trie == NULL) break;

    matcher = malloc(sizeof(struct ambi_matcher));
    if (matcher == NULL) break;

    matcher->_dict = dict_retain(dict);

    matcher->_pure_matcher = (matcher_t)
        dat_construct_by_trie(trie, enable_automation);
    matcher->_pure_matcher->_func = dat_matcher_func;
    matcher->_pure_matcher->_type =
        enable_automation ? matcher_type_acdat : matcher_type_dat;
    trie_destruct(trie);

    return matcher;
  } while(0);

  // clean
  trie_destruct(trie);

  return matcher;
}

ambi_matcher_t ambi_construct(vocab_t vocab,
                              mdi_prop_f filter,
                              bool enable_automation) {
  ambi_matcher_t ambi_matcher = NULL;
  match_dict_t dict = NULL;

  if (vocab == NULL) return NULL;

  dict = dict_alloc();
  if (dict == NULL) return NULL;
  dict->add_index_filter =
      dict_add_index_filter_wrap(dict->add_index_filter, ambi_dict_add_index);

  if (dict_parse(dict, vocab)) {
    ambi_matcher = ambi_construct_by_dict(dict, filter, enable_automation);
  }

  dict_release(dict);

  fprintf(stderr,
          "construct trie %s!\n",
          ambi_matcher != NULL ? "success" : "failed");

  return ambi_matcher;
}

bool ambi_destruct(ambi_matcher_t self) {
  return false;
}

ambi_context_t ambi_alloc_context(ambi_matcher_t matcher) {
  return NULL;
}

bool ambi_free_context(ambi_context_t context) {
  return false;
}

bool ambi_reset_context(ambi_context_t context,
                        unsigned char content[],
                        size_t len) {
  return false;
}

int ambi_map_cmp_data(void *a, void *b) {
  mdiq_node_t na = a, nb = b;
  if (na->idx->_tag == nb->idx->_tag) return 0;
  return na->idx->_tag < nb->idx->_tag ? -1 : 1;
}

void ambi_map_set_data(void **container, void *data) {
  mdiq_node_t new = data;
  if (*container == NULL) {
    new->mqn_next = NULL;
  } else {
    new->mqn_next = *container;
  }
  *container = data;
}

bool ambi_next_on_index(ambi_context_t ctx) {

  mdiq_node_s helper;
  context_t pctx = ctx->_pure_ctx;

  // get matched sequence
  while (matcher_next(pctx)) {
    match_dict_index_t mdi = matcher_matched_index(pctx);
    helper.idx = mdi;
    if (mdi->prop & mdi_prop_normal) {
      // matched word, check last ambi for w-l
      mdiq_node_t last_ambi = avl_search(ctx->_ambi_map, &helper);
      if (last_ambi == NULL
          || !strpos_wlc(matcher_matched_pos(pctx), last_ambi->pos)) {
        // don't match ambi present, add word to map
        mdiq_node_t node = (mdiq_node_t) dynapool_alloc_node(ctx->_mdiqn_pool);
        node->pos = matcher_matched_pos(pctx);
        node->idx = matcher_matched_index(pctx);
        avl_insert(ctx->_word_map, node);
        deque_push_front(&ctx->_out_buffer, node, mdiq_node_s, deque_elem);
      }
    } else if (mdi->prop & mdi_prop_ambi) {
      // matched ambi, check word list by desc order for w-r and w-c
      mdiq_node_t last_word = avl_search(ctx->_word_map, &helper);
      while (last_word != NULL) {
        if (strpos_wrc(last_word->pos, matcher_matched_pos(pctx))) {
          // matched ambi, delete word
          // TODO: remove from map
          deque_delete(&ctx->_out_buffer, last_word, mdiq_node_s, deque_elem);
          dynapool_free_node(ctx->_mdiqn_pool, (dynapool_node_t) last_word);
        }
        if (last_word->pos.eo <= pctx->out_eo-mdi->length) break;
        last_word = (mdiq_node_t) (last_word->mqn_next);
      }
    }
  }

  return false;
}

//int main() {
//  size_t key_cur, head_cur, tail_cur;
//  size_t tag, distance;
//  regmatch_t pmatch[12];
//  char dist[3], *split;
//  int err;
//
//  if (!pattern_compiled) {
//    // compile pattern
//    err = regcomp(&reg, pattern, REG_EXTENDED | REG_NEWLINE);
//    if (err) {
//      fprintf(stderr, "error: compile regex failed!\n");
//      return false;
//    } else {
//      pattern_compiled = true;
//    }
//  }
//
//  err = regexec(&reg, "1234(?&!hhh|aaa)", 12, pmatch, 0);
//  if (err == REG_NOMATCH) {
//    return true;
//  } else if (err != REG_NOERROR) {
//    return false;
//  }
//
//  return 0;
//}
