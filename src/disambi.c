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
                         void *tag,
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
    trie_config_s config = {.filter=filter, enable_automation=false};
    trie = trie_construct_by_dict(dict, &config);
    if (trie == NULL) break;

    matcher = amalloc(sizeof(struct ambi_matcher));
    if (matcher == NULL) break;

    matcher->_dict = dict_retain(dict);

    matcher->_pure_matcher = (matcher_t)
        dat_construct_by_trie(trie, enable_automation);
    matcher->_pure_matcher->_func = dat_matcher_func;
    matcher->_pure_matcher->_type =
        enable_automation ? matcher_type_acdat : matcher_type_dat;
    trie_destruct(trie);

    return matcher;
  } while (0);

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
      dict_add_index_filter_wrap(dict->add_index_filter, dict_add_alternation_index);
  dict->add_index_filter =
      dict_add_index_filter_wrap(dict->add_index_filter, ambi_dict_add_index);

  if (dict_parse(dict, vocab)) {
    ambi_matcher = ambi_construct_by_dict(dict, filter, enable_automation);
  }

  dict_release(dict);

  fprintf(stderr, "construct disambi %s!\n",
          ambi_matcher != NULL ? "success" : "failed");

  return ambi_matcher;
}

bool ambi_destruct(ambi_matcher_t self) {
  if (self == NULL) return false;
  matcher_destruct(self->_pure_matcher);
  afree(self);
  return true;
}

ambi_context_t ambi_alloc_context(ambi_matcher_t matcher) {
  ambi_context_t ctx = NULL;

  do {
    ctx = amalloc(sizeof(ambi_context_s));
    if (ctx == NULL) break;

    ctx->_matcher = matcher;
    ctx->_pure_ctx = NULL;

    ctx->_mdiqn_pool = NULL;
    ctx->_word_map = NULL;
    ctx->_ambi_map = NULL;

    deque_init(&ctx->_out_buffer);

    ctx->_pure_ctx = matcher_alloc_context(matcher->_pure_matcher);
    if (ctx->_pure_ctx == NULL) break;

    ctx->_mdiqn_pool = dynapool_construct_with_type(mdim_node_s);
    if (ctx->_mdiqn_pool == NULL) break;

    ctx->_word_map = mdimap_construct(true);
    if (ctx->_word_map == NULL) break;
    ctx->_ambi_map = mdimap_construct(true);
    if (ctx->_ambi_map == NULL) break;

    return ctx;
  } while (0);

  ambi_free_context(ctx);

  return NULL;
}

bool ambi_free_context(ambi_context_t context) {
  if (context == NULL) return false;
  mdimap_destruct(context->_word_map);
  mdimap_destruct(context->_ambi_map);
  dynapool_destruct(context->_mdiqn_pool);
  matcher_free_context(context->_pure_ctx);
  afree(context);
  return true;
}

bool ambi_reset_context(ambi_context_t ctx,
                        unsigned char content[],
                        size_t len) {
  ctx->header.content = content;
  ctx->header.len = len;
  ctx->header.out_matched_index = NULL;
  ctx->header.out_eo = 0;

  matcher_reset_context(ctx->_pure_ctx, (char *) content, len);

  mdimap_reset(ctx->_word_map);
  mdimap_reset(ctx->_ambi_map);
  dynapool_reset(ctx->_mdiqn_pool);
  deque_init(&ctx->_out_buffer);

  return true;
}

bool ambi_next_on_index(ambi_context_t ctx) {
  context_t pctx = ctx->_pure_ctx;

  // not cache
  if (deque_empty(&ctx->_out_buffer)) {
    mdimap_reset(ctx->_word_map);
    mdimap_reset(ctx->_ambi_map);
    dynapool_reset(ctx->_mdiqn_pool);

    // get matched sequence
    while (matcher_next(pctx)) {
      mdi_t mdi = matcher_matched_index(pctx);
      if (mdi->prop & mdi_prop_normal) {
        // matched word, check last ambi for w-l
        mdim_node_t last_ambi = mdimap_search(ctx->_ambi_map, mdi->_tag);
        if (last_ambi == NULL
            || !strpos_wlc(matcher_matched_pos(pctx), last_ambi->pos)) {
          // don't match ambi present, add word to map
          mdim_node_t node = (mdim_node_t) dynapool_alloc_node(ctx->_mdiqn_pool);
          node->pos = matcher_matched_pos(pctx);
          node->idx = matcher_matched_index(pctx);
          mdimap_insert(ctx->_word_map, node);
          deque_push_back(&ctx->_out_buffer, node, mdim_node_s, deque_elem);
        }
      } else if (mdi->prop & mdi_prop_ambi) {
        // matched ambi, check word list by desc order for w-r and w-c
        mdim_node_t last_word = mdimap_search(ctx->_word_map, mdi->_tag);
        while (last_word != NULL) {
          if (strpos_wrc(last_word->pos, matcher_matched_pos(pctx))) {
            // matched ambi, delete word
            // 1. remove from map
            mdimap_delete(ctx->_word_map, last_word);
            // 2. remove from queue
            deque_delete(&ctx->_out_buffer, last_word, mdim_node_s, deque_elem);
            // 3. free memory
            dynapool_free_node(ctx->_mdiqn_pool, last_word);
          }
          if (last_word->pos.eo <= pctx->out_eo - mdi->length) break;
          last_word = last_word->next;
        }
        mdim_node_t node = (mdim_node_t) dynapool_alloc_node(ctx->_mdiqn_pool);
        node->pos = matcher_matched_pos(pctx);
        node->idx = matcher_matched_index(pctx);
        mdimap_insert(ctx->_ambi_map, node);
      }
    }

    // not matched
    if (deque_empty(&ctx->_out_buffer)) return false;
  }

  // pop from queue
  mdim_node_t node = deque_pop_front(&ctx->_out_buffer, mdim_node_s, deque_elem);
  ctx->header.out_matched_index = node->idx;
  ctx->header.out_eo = node->pos.eo;

  return true;
}
