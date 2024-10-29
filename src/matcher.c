/**
 * matcher.c
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#include "actrie/matcher.h"

#include "parser/parser.h"
#include "reglet/engine.h"
#include "reglet/expr/expr.h"
#include "trie/acdat.h"

typedef struct _actrie_matcher_ {
  dat_t datrie;
  reglet_t reglet;
  segarray_t extra_store;  // keep reference of extra strings.
} matcher_s;

static matcher_t matcher_alloc() {
  matcher_t matcher = amalloc(sizeof(matcher_s));
  matcher->datrie = NULL;
  matcher->reglet = NULL;
  matcher->extra_store = NULL;
  return matcher;
}

static void matcher_free(matcher_t matcher) {
  afree(matcher);
}

typedef struct _add_pattern_params_ {
  matcher_t matcher;
  trie_t extra_trie;
} add_pattern_params_s, *add_pattern_params_t;

static void add_pattern_to_matcher(ptrn_t pattern, strlen_t extra, void* arg) {
  add_pattern_params_t args = (add_pattern_params_t)arg;
  dstr_t ds = NULL;
  if (extra->len > 0) {
    if (args->extra_trie != NULL) {
      ds = trie_search(args->extra_trie, extra->ptr, extra->len);
    }
    if (ds == NULL) {
      size_t alloced = segarray_size(args->matcher->extra_store);
      if (segarray_extend(args->matcher->extra_store, 1) != 1) {
        // alloc failed!!!
        exit(-1);
      }
      dstr_t* dsc = (dstr_t*)segarray_access(args->matcher->extra_store, alloced);
      ds = *dsc = dstr(extra);
      if (args->extra_trie != NULL) {
        trie_add_keyword(args->extra_trie, extra->ptr, extra->len, ds);
      }
    }
  }
  reglet_add_pattern(args->matcher->reglet, pattern, ds);
}

static void expr_list_free(void* trie, void* node) {
  list_t list = (list_t)node;
  while (list != NULL) {
    // NOTE: car of list is expr_t, and it is not aobj!!!
    list->car = NULL;
    list = list->cdr;
  }
  list = (list_t)node;
  _release(list);
}

static matcher_t matcher_construct(vocab_t vocab,
                                   bool all_as_plain,
                                   bool ignore_bad_pattern,
                                   bool bad_as_plain,
                                   bool deduplicate_extra,
                                   segarray_config_t extra_store_config) {
  trie_t extra_trie = deduplicate_extra ? trie_alloc() : NULL;

  // create matcher
  matcher_t matcher = matcher_alloc();
  matcher->extra_store =
      extra_store_config == NULL
          ? segarray_construct_with_type(dstr_t)
          : segarray_construct_with_type_ext(dstr_t, extra_store_config->seg_blen, extra_store_config->region_size);
  matcher->reglet = reglet_construct();

  // load vocabulary
  add_pattern_params_s add_pattern_args = {.matcher = matcher, .extra_trie = extra_trie};
  if (!parse_vocab(vocab, add_pattern_to_matcher, &add_pattern_args, all_as_plain, ignore_bad_pattern, bad_as_plain)) {
    // release trie manually
    trie_free(matcher->reglet->trie, (trie_node_free_f)expr_list_free);
    matcher->reglet->trie = NULL;
    matcher_destruct(matcher);
    matcher = NULL;
  }

  if (matcher != NULL) {
    // build datrie by reglet->trie
    trie_sort_to_bfs(matcher->reglet->trie);
    matcher->datrie = dat_construct_by_trie(matcher->reglet->trie, true);
    // then, free reglet->trie
    trie_free(matcher->reglet->trie, NULL);
    matcher->reglet->trie = NULL;
  }

  // free extra_trie
  if (extra_trie) {
    trie_free(extra_trie, NULL);
  }

  return matcher;
}

matcher_t matcher_construct_by_file(const char* path,
                                    bool all_as_plain,
                                    bool ignore_bad_pattern,
                                    bool bad_as_plain,
                                    bool deduplicate_extra) {
  return matcher_construct_by_file_ext(path, all_as_plain, ignore_bad_pattern, bad_as_plain, deduplicate_extra, NULL);
}

matcher_t matcher_construct_by_file_ext(const char* path,
                                        bool all_as_plain,
                                        bool ignore_bad_pattern,
                                        bool bad_as_plain,
                                        bool deduplicate_extra,
                                        segarray_config_t extra_store_config) {
  vocab_t vocab = vocab_construct(stream_type_file, (void*)path);
  if (vocab == NULL) {
    return NULL;
  }

  matcher_t matcher =
      matcher_construct(vocab, all_as_plain, ignore_bad_pattern, bad_as_plain, deduplicate_extra, extra_store_config);
  vocab_destruct(vocab);
  return matcher;
}

matcher_t matcher_construct_by_string(strlen_t string,
                                      bool all_as_plain,
                                      bool ignore_bad_pattern,
                                      bool bad_as_plain,
                                      bool deduplicate_extra) {
  return matcher_construct_by_string_ext(string, all_as_plain, ignore_bad_pattern, bad_as_plain, deduplicate_extra,
                                         NULL);
}

matcher_t matcher_construct_by_string_ext(strlen_t string,
                                          bool all_as_plain,
                                          bool ignore_bad_pattern,
                                          bool bad_as_plain,
                                          bool deduplicate_extra,
                                          segarray_config_t extra_store_config) {
  vocab_t vocab = vocab_construct(stream_type_string, string);
  matcher_t matcher =
      matcher_construct(vocab, all_as_plain, ignore_bad_pattern, bad_as_plain, deduplicate_extra, extra_store_config);
  vocab_destruct(vocab);
  return matcher;
}

static void extra_store_free(segarray_t extra_store) {
  if (extra_store != NULL) {
    size_t store_size = segarray_size(extra_store);
    for (size_t idx = 0; idx < store_size; idx++) {
      dstr_t ds = *(dstr_t*)segarray_access(extra_store, idx);
      _release(ds);
    }
    segarray_destruct(extra_store);
  }
}

void matcher_destruct(matcher_t matcher) {
  if (matcher != NULL) {
    dat_destruct(matcher->datrie, (dat_node_free_f)expr_list_free);
    reglet_destruct(matcher->reglet);
    extra_store_free(matcher->extra_store);
    matcher_free(matcher);
  }
}

typedef struct _actrie_context_ {
  strlen_s content;
  reg_ctx_t reg_ctx;
  dat_ctx_t dat_ctx;
  word_s matched_word;
} context_s;

static context_t context_alloc() {
  context_t context = amalloc(sizeof(context_s));
  context->content.ptr = NULL;
  context->content.len = 0;
  context->reg_ctx = NULL;
  context->dat_ctx = NULL;
  return context;
}

static void context_free(context_t context) {
  afree(context);
}

context_t matcher_alloc_context(matcher_t matcher) {
  context_t context = context_alloc();
  context->dat_ctx = dat_alloc_context(matcher->datrie);
  context->reg_ctx = reglet_alloc_context(matcher->reglet);
  return context;
}

void matcher_free_context(context_t context) {
  if (context != NULL) {
    dat_free_context(context->dat_ctx);
    reglet_free_context(context->reg_ctx);
    context_free(context);
  }
}

void matcher_fix_pos(context_t context, fix_pos_f fix_pos_func, void* fix_pos_arg) {
  reglet_fix_pos(context->reg_ctx, fix_pos_func, fix_pos_arg);
}

void matcher_reset_context(context_t context, char content[], size_t len) {
  context->content = (strlen_s){.ptr = content, .len = len};
  dat_reset_context(context->dat_ctx, content, len);
  reglet_reset_context(context->reg_ctx, content, len);
}

typedef bool (*dat_next_on_node_f)(dat_ctx_t ctx);

static word_t matcher_next0(context_t context, dat_next_on_node_f dat_next_on_node_func) {
  // 不保证输出有序
  pos_cache_t matched = prique_pop(context->reg_ctx->output_queue);
  if (matched == NULL) {
    while (dat_next_on_node_func(context->dat_ctx)) {
      list_t expr_list = dat_matched_value(context->dat_ctx);
      while (expr_list != NULL) {
        expr_t expr = _(list, expr_list, car);
        pos_cache_t pos_cache = dynapool_alloc_node(context->reg_ctx->pos_cache_pool);
        // datrie only output end offset, and set start offset in expr_text
        pos_cache->pos.eo = context->dat_ctx->_read;
        expr_feed_text(expr, pos_cache, context->reg_ctx);
        expr_list = _(list, expr_list, cdr);
      }
      matched = prique_pop(context->reg_ctx->output_queue);
      if (matched != NULL) {
        break;
      }
    }
  }
  if (matched == NULL) {
    reglet_activate_expr_ctx(context->reg_ctx);
    matched = prique_pop(context->reg_ctx->output_queue);
  }
  if (matched != NULL) {
    // matche pattern, output
    context->matched_word.keyword =
        (strlen_s){.ptr = context->content.ptr + matched->pos.so, .len = matched->pos.eo - matched->pos.so};
    if (matched->embed.extra != NULL) {
      context->matched_word.extra = (strlen_s){.ptr = matched->embed.extra->str, .len = matched->embed.extra->len};
    } else {
      context->matched_word.extra = strlen_empty;
    }
    context->matched_word.pos = matched->pos;
    dynapool_free_node(context->reg_ctx->pos_cache_pool, matched);
    return &context->matched_word;
  }
  return NULL;
}

word_t matcher_next(context_t context) {
  return matcher_next0(context, dat_ac_next_on_node);
}

word_t matcher_next_prefix(context_t context) {
  return matcher_next0(context, dat_ac_prefix_next_on_node);
}
