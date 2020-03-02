/**
 * matcher.c
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#include "matcher.h"

#include "parser/parser.h"
#include "reglet/engine.h"
#include "reglet/expr/expr.h"
#include "trie/acdat.h"

typedef struct _actrie_matcher_ {
  reglet_t reglet;
  dat_t datrie;
} matcher_s;

static matcher_t matcher_alloc() {
  matcher_t matcher = amalloc(sizeof(matcher_s));
  matcher->reglet = NULL;
  matcher->datrie = NULL;
  return matcher;
}

static void matcher_free(matcher_t matcher) {
  afree(matcher);
}

static void add_pattern_to_matcher(ptrn_t pattern, strlen_t extra, void* arg) {
  matcher_t matcher = (matcher_t)arg;
  reglet_add_pattern(matcher->reglet, pattern, extra);
}

static void free_expr_list(void* trie, void* node) {
  list_t list = (list_t)node;
  _release(list);
}

matcher_t matcher_construct(vocab_t vocab) {
  matcher_t matcher = matcher_alloc();

  // build reglet
  matcher->reglet = reglet_construct();
  if (!parse_vocab(vocab, add_pattern_to_matcher, matcher, false)) {
    trie_free(matcher->reglet->trie, (trie_node_free_f)free_expr_list);
    matcher->reglet->trie = NULL;
    matcher_destruct(matcher);
    return NULL;
  }

  // build datrie by reglet->trie
  trie_sort_to_line(matcher->reglet->trie);
  matcher->datrie = dat_construct_by_trie(matcher->reglet->trie, true);
  // then, free reglet->trie
  trie_free(matcher->reglet->trie, NULL);
  matcher->reglet->trie = NULL;

  return matcher;
}

matcher_t matcher_construct_by_file(const char* path) {
  vocab_t vocab = vocab_construct(stream_type_file, (void*)path);
  matcher_t matcher = matcher_construct(vocab);
  vocab_destruct(vocab);
  return matcher;
}

matcher_t matcher_construct_by_string(strlen_t string) {
  vocab_t vocab = vocab_construct(stream_type_string, string);
  matcher_t matcher = matcher_construct(vocab);
  vocab_destruct(vocab);
  return matcher;
}

void matcher_destruct(matcher_t matcher) {
  if (matcher != NULL) {
    reglet_destruct(matcher->reglet);
    dat_destruct(matcher->datrie, (dat_node_free_f)free_expr_list);
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
  reglet_reset_context(context->reg_ctx);
}

typedef bool (*dat_next_on_node_f)(dat_ctx_t ctx);

static word_t matcher_next0(context_t context, dat_next_on_node_f dat_next_on_node_func) {
  // 不保证输出有序
  pos_cache_t matched = prique_pop(context->reg_ctx->output_queue);
  if (matched == NULL) {
    while (dat_next_on_node_func(context->dat_ctx)) {
      list_t expr_list = context->dat_ctx->_value;
      while (expr_list != NULL) {
        expr_t expr = _(list, expr_list, car);
        pos_cache_t pos_cache = dynapool_alloc_node(context->reg_ctx->pos_cache_pool);
        // datrie only output end offset, and set start offset in expr_text
        pos_cache->pos.eo = context->dat_ctx->_e;
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
    activate_ambi_queue(context->reg_ctx);
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
  return matcher_next0(context, dat_prefix_next_on_node);
}
