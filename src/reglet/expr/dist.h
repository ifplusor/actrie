/**
 * dist.h
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#ifndef __ACTRIE_EXPR_DIST_H__
#define __ACTRIE_EXPR_DIST_H__

#include "expr0.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _regex_expression_distance_ {
  expr_s header;
  uint32_t min, max;
} expr_dist_s, *expr_dist_t;

void expr_init_dist(expr_dist_t self, expr_t target, expr_feed_f feed, uint32_t min, uint32_t max);

void expr_feed_dist_prefix(expr_t self, pos_cache_t prefix, reg_ctx_t context);
void expr_feed_dist_suffix(expr_t self, pos_cache_t suffix, reg_ctx_t context);

void expr_feed_ddist_prefix(expr_t self, pos_cache_t prefix, reg_ctx_t context);
void expr_feed_ddist_suffix(expr_t self, pos_cache_t suffix, reg_ctx_t context);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  // __ACTRIE_EXPR_DIST_H__
