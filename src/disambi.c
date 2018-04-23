//
// Created by james on 9/27/17.
//

#include "disambi.h"
#include "acdat.h"

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
static const char *pattern = "^(.*)\x01\x06(.*)\x02$";

bool ambi_dict_add_index(match_dict_t dict, matcher_conf_t config, strlen_s keyword,
                         strlen_s extra, void *tag, mdi_prop_f prop) {
  ambi_conf_t ambi_conf = (ambi_conf_t) config->buf;
  matcher_conf_t pure_conf = ambi_conf->pure;

  if (ambi_conf->regex == NULL) {
    // compile pattern
    const char *errorptr;
    int errorcode;
    int erroffset;
    ambi_conf->regex = pcre_compile2(pattern, PCRE_DOTALL | PCRE_UTF8, &errorcode, &errorptr, &erroffset, NULL);
    if (ambi_conf->regex == NULL) {
      ALOG_FATAL(errorptr);
    }
  }

  if (keyword.len == 0) return true;

  int ovector[9];
  int rc = pcre_exec(ambi_conf->regex, NULL, keyword.ptr, (int) keyword.len, 0, 0, ovector, 9);
  if (rc == PCRE_ERROR_NOMATCH) {
    // non-ambi
    pure_conf->add_index(dict, pure_conf, keyword, extra, tag,
                         mdi_prop_clearly | prop);
    return true;
  } else if (rc < 0) {
    return false;
  }

  mdi_prop_f base_prop = mdi_prop_tag_id | mdi_prop_bufkey;

  // store original keyword
  void *key_tag = (void *) dict->idx_count;
  dict_add_index(dict, config, keyword, extra, tag, prop);

  // store processed keyword
  strlen_s key = {
      .ptr = keyword.ptr + PCRE_VEC_SO(ovector, 1),
      .len = (size_t) (PCRE_VEC_EO(ovector, 1) - PCRE_VEC_SO(ovector, 1)),
  };

  strlen_s ambi = {
      .ptr = keyword.ptr + PCRE_VEC_SO(ovector, 2),
      .len = (size_t) (PCRE_VEC_EO(ovector, 2) - PCRE_VEC_SO(ovector, 2)),
  };

  pure_conf->add_index(dict, pure_conf, key, strlen_empty, key_tag,
                       mdi_prop_normal | base_prop);

  pure_conf->add_index(dict, pure_conf, ambi, strlen_empty, key_tag,
                       mdi_prop_ambi | base_prop);

  return true;
}

void ambi_config_clean(matcher_conf_t config) {
  if (config != NULL) {
    ambi_conf_t ambi_config = (ambi_conf_t) config->buf;
    _release(ambi_config->pure);
    free(ambi_config->regex);
  }
}

matcher_conf_t ambi_matcher_conf(uint8_t id, matcher_conf_t pure) {
  matcher_conf_t config = matcher_conf(id, matcher_type_ambi, ambi_dict_add_index,
                                       sizeof(ambi_conf_s));
  if (config) {
    config->clean = ambi_config_clean;
    ambi_conf_t ambi_config = (ambi_conf_t) config->buf;
    ambi_config->pure = pure;
    ambi_config->regex = NULL;
  }
  return config;
}

matcher_t ambi_construct(match_dict_t dict, matcher_conf_t conf) {
  ambi_conf_t ambi_conf = (ambi_conf_t) conf->buf;
  ambi_matcher_t matcher = NULL;
  matcher_t pure_matcher = NULL;

  do {
    pure_matcher = matcher_construct_by_dict(dict, ambi_conf->pure);
    if (pure_matcher == NULL) break;

    matcher = amalloc(sizeof(struct ambi_matcher));
    if (matcher == NULL) break;

    matcher->_dict = dict_retain(dict);

    matcher->_pure_matcher = pure_matcher;

    matcher->header._type = conf->type;
    matcher->header._func = ambi_matcher_func;

    return (matcher_t) matcher;
  } while (0);

  // clean
  matcher_destruct(pure_matcher);

  return NULL;
}

bool ambi_destruct(ambi_matcher_t self) {
  if (self == NULL) return false;
  matcher_destruct(self->_pure_matcher);
  dict_release(self->_dict);
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

    ctx->hdr._type = matcher->header._type;
    ctx->hdr._func = ambi_context_func;

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

bool ambi_reset_context(ambi_context_t ctx, char content[], size_t len) {
  ctx->hdr.content = (strlen_s) {.ptr = content, .len = len};

  ctx->hdr.out_index = NULL;
  ctx->hdr.out_pos = (strpos_s) {.so = 0, .eo = 0};

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
    // optimize for useless reset
    if (dat_match_end((dat_context_t) pctx)) return false;

    mdimap_reset(ctx->_word_map);
    mdimap_reset(ctx->_ambi_map);
    dynapool_reset(ctx->_mdiqn_pool);

    int normal = 0;
    while (1) {
      // get matched sequence
      while (matcher_next(pctx)) {
        mdi_t mdi = matcher_matched_index(pctx);
        if (mdi->prop & mdi_prop_clearly) {
          mdim_node_t
              node = (mdim_node_t) dynapool_alloc_node(ctx->_mdiqn_pool);
          node->pos = matcher_matched_pos(pctx);
          node->idx = matcher_matched_index(pctx);
          deque_push_back(&ctx->_out_buffer, node, mdim_node_s, deque_elem);
        } else if (mdi->prop & mdi_prop_normal) {
          // matched word, check last ambi for w-l
          mdim_node_t last_ambi = mdimap_search(ctx->_ambi_map, mdi->_tag);
          if (last_ambi == NULL
              || !strpos_wlc(matcher_matched_pos(pctx), last_ambi->pos)) {
            // don't match ambi present, add word to map
            mdim_node_t
                node = (mdim_node_t) dynapool_alloc_node(ctx->_mdiqn_pool);
            node->pos = matcher_matched_pos(pctx);
            node->idx = matcher_matched_index(pctx);
            mdimap_insert(ctx->_word_map, node);
            deque_push_back(&ctx->_out_buffer, node, mdim_node_s, deque_elem);
            normal++;
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
              deque_delete(&ctx->_out_buffer,
                           last_word,
                           mdim_node_s,
                           deque_elem);
              // 3. free memory
              dynapool_free_node(ctx->_mdiqn_pool, last_word);
              normal--;
            }
            if (last_word->pos.eo <= matcher_matched_pos(pctx).so) break;
            last_word = last_word->next;
          }
          mdim_node_t
              node = (mdim_node_t) dynapool_alloc_node(ctx->_mdiqn_pool);
          node->pos = matcher_matched_pos(pctx);
          node->idx = matcher_matched_index(pctx);
          mdimap_insert(ctx->_ambi_map, node);
        }
      }

      if (normal > 0) break;

      // not matched
      if (dat_match_end((dat_context_t) pctx)) {
        if (deque_empty(&ctx->_out_buffer))
          return false;
        else
          break;
      }
    }
  }

  // pop from queue
  mdim_node_t node = deque_pop_front(&ctx->_out_buffer, mdim_node_s, deque_elem);
  if (node->idx->prop & mdi_prop_clearly) {
    ctx->hdr.out_index = node->idx;
    ctx->hdr.out_pos = node->pos;
  } else {
    ctx->hdr.out_index = node->idx->_tag;
    ctx->hdr.out_pos = node->pos;
  }

  return true;
}
