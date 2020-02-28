/**
 * text.c
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#include "text.h"

void expr_init_text(expr_text_t self, expr_t target, expr_feed_f feed, size_t len) {
  expr_init(&self->header, target, feed);
  self->len = len;
}

void expr_feed_text(expr_t expr, pos_cache_t keyword, void* context) {
  expr_text_t self = container_of(expr, expr_text_s, header);
  // calculate start offset
  keyword->pos.so = keyword->pos.eo - self->len;
  expr_feed_target(expr, keyword, context);
}
