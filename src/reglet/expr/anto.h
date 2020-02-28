/**
 * anto.h
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#ifndef __ACTRIE_EXPR_ANTO__
#define __ACTRIE_EXPR_ANTO__

#include "expr0.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _regex_expression_anti_antonym_ {
  expr_s header;
} expr_anto_s, *expr_anto_t;

void expr_init_anto(expr_anto_t self, expr_t target, expr_feed_f feed);

void expr_feed_anto_antonym(expr_t self, pos_cache_t antonym, reg_ctx_t context);
void expr_feed_anto_center(expr_t self, pos_cache_t center, reg_ctx_t context);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  // __ACTRIE_EXPR_ANTO__
