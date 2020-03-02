/**
 * utf8helper.h
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#ifndef __ACTRIE_UTF8_HELPER_H__
#define __ACTRIE_UTF8_HELPER_H__

#include <alib/acom.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _actrie_utf8_context_ {
  size_t* pos;
  size_t len;
} utf8_ctx_s, *utf8_ctx_t;

utf8_ctx_t alloc_utf8_context();
void free_utf8_context(utf8_ctx_t context);

bool reset_utf8_context(utf8_ctx_t context, char content[], size_t len);

size_t fix_utf8_pos(size_t pos, size_t diff, bool plus_or_subtract, void* arg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  // __ACTRIE_UTF8_HELPER_H__
