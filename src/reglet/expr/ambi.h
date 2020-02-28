/**
 * ambi.h
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#ifndef __ACTRIE_EXPR_AMBI__
#define __ACTRIE_EXPR_AMBI__

#include "expr0.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _regex_expression_anti_ambiguity_ {
  expr_s header;
} expr_ambi_s, *expr_ambi_t;

void expr_init_ambi(expr_ambi_t self, expr_t target, expr_feed_f feed);

void expr_feed_ambi_ambiguity(expr_t self, pos_cache_t ambiguity, reg_ctx_t context);
void expr_feed_ambi_center(expr_t self, pos_cache_t center, reg_ctx_t context);

void activate_ambi_queue(reg_ctx_t context);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  // __ACTRIE_EXPR_AMBI__
