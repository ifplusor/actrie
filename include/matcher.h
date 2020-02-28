/**
 * matcher.h
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#ifndef __ACTRIE_MATCHER_H__
#define __ACTRIE_MATCHER_H__

#include <alib/string/astr.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// matcher API
// ==============

struct _actrie_matcher_;
typedef struct _actrie_matcher_* matcher_t;

struct _actrie_context_;
typedef struct _actrie_context_* context_t;

matcher_t matcher_construct_by_file(const char* path);
matcher_t matcher_construct_by_string(strlen_t string);
void matcher_destruct(matcher_t matcher);

context_t matcher_alloc_context(matcher_t matcher);
void matcher_free_context(context_t context);

bool matcher_reset_context(context_t context, char content[], size_t len);

typedef struct _actrie_matched_word_ {
  strlen_s keyword;
  strlen_s extra;
  strpos_s pos;
} word_s, *word_t;

word_t matcher_next(context_t context);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  // __ACTRIE_MATCHER_H__
