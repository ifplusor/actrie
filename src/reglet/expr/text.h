/**
 * text.h
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#ifndef __ACTRIE_EXPR_TEXT_H__
#define __ACTRIE_EXPR_TEXT_H__

#include "expr0.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _regex_exprerssion_text_ {
  expr_s header;
  size_t len;
} expr_text_s, *expr_text_t;

void expr_init_text(expr_text_t self, expr_t target, expr_feed_f feed, size_t len);

void expr_feed_text(expr_t self, pos_cache_t keyword, void* context);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  // __ACTRIE_EXPR_TEXT_H__
