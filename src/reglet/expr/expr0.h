/**
 * expr0.h
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#ifndef __ACTRIE_EXPR_FEED_H__
#define __ACTRIE_EXPR_FEED_H__

#include <alib/string/astr.h>

#include "../context.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct _regex_expression_;
typedef struct _regex_expression_ expr_s;
typedef expr_s* expr_t;

struct _expression_context_;
typedef struct _expression_context_ expr_ctx_s;
typedef expr_ctx_s* expr_ctx_t;

//
// expression

/**
 * expr_feed_f - feed keyword to expression
 * @param expr - target expersion
 * @param keyword
 * @return true if matched pattern
 */
typedef void (*expr_feed_f)(expr_t expr, pos_cache_t keyword, reg_ctx_t context);

struct _regex_expression_ {
  expr_t target;
  expr_feed_f target_feed;
};

inline void expr_init(expr_t self, expr_t target, expr_feed_f feed) {
  self->target = target;
  self->target_feed = feed;
}

inline void expr_feed_target(expr_t self, pos_cache_t keyword, reg_ctx_t context) {
  self->target_feed(self->target, keyword, context);
}

typedef struct _expr_feed_arg_ {
  expr_t expr;
  pos_cache_t keyword;
  reg_ctx_t context;
} expr_feed_arg_s, *expr_feed_arg_t;

//
// expression context

typedef void (*expr_ctx_free_f)(expr_ctx_t expr_ctx, reg_ctx_t reg_ctx);
typedef void (*expr_ctx_activate_f)(expr_ctx_t expr_ctx, reg_ctx_t reg_ctx);

struct _expression_context_ {
  expr_t expr;
  expr_ctx_free_f free_func;
  expr_ctx_activate_f activate_func;
  avl_node_s avl_elem;
};

inline void expr_ctx_init(expr_ctx_t self, expr_t expr, expr_ctx_free_f free, expr_ctx_activate_f activate) {
  self->expr = expr;
  self->free_func = free;
  self->activate_func = activate;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  // __ACTRIE_EXPR_FEED_H__
