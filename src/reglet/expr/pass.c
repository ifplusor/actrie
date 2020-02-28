/**
 * pass.c
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#include "pass.h"

void expr_init_pass(expr_pass_t self, expr_t target, expr_feed_f feed) {
  expr_init(&self->header, target, feed);
}

void expr_feed_pass(expr_t self, pos_cache_t keyword, reg_ctx_t context) {
  expr_feed_target(self, keyword, context);
}
