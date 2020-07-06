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
  strlen_s content;
  context_t matcher_ctx;
  utf8_ctx_s utf8_ctx;
  bool return_byte_pos;
} utf8ctx_s, *utf8ctx_t;

utf8ctx_t utf8ctx_alloc_context(matcher_t matcher);
void utf8ctx_free_context(utf8ctx_t utf8ctx);
bool utf8ctx_reset_context(utf8ctx_t utf8ctx, char* content, int length, bool return_byte_pos);
word_t utf8ctx_next(utf8ctx_t utf8ctx);
word_t utf8ctx_next_prefix(utf8ctx_t utf8ctx);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  // __ACTRIE_UTF8POS_H__
