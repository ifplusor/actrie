/**
 * pass.h
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#ifndef __ACTRIE_EXPR_PASS__
#define __ACTRIE_EXPR_PASS__

#include "expr0.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _regex_exprerssion_pass_ {
  expr_s header;
} expr_pass_s, *expr_pass_t;

void expr_init_pass(expr_pass_t self, expr_t target, expr_feed_f feed);

void expr_feed_pass(expr_t self, pos_cache_t keyword, reg_ctx_t context);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  // __ACTRIE_EXPR_PASS__
