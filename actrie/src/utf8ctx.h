/**
 * utf8ctx.h
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#ifndef __ACTRIE_UTF8POS_H__
#define __ACTRIE_UTF8POS_H__

#include <matcher.h>
#include <utf8helper.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct context_wrapper {
  context_t matcher_ctx;
  utf8_ctx_s utf8_ctx;
} wctx_s, *wctx_t;

wctx_t alloc_context(matcher_t matcher);
void free_context(wctx_t wctx);
bool reset_context(wctx_t wctx, char* content, int length);
word_t next(wctx_t wctx);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  // __ACTRIE_UTF8POS_H__
