/**
 * engine.h
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#ifndef __ACTRIE_REGEX_ENGINE_H__
#define __ACTRIE_REGEX_ENGINE_H__

#include "../pattern.h"
#include "../trie/actrie.h"
#include "context.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _regex_applet_ {
  dynapool_t expr_pool;
  trie_t extra_store;
  trie_t trie;
} reglet_s, *reglet_t;

reglet_t reglet_construct();
void reglet_destruct(reglet_t reglet);

void reglet_add_pattern(reglet_t self, ptrn_t pattern, strlen_t extra);

reg_ctx_t reglet_alloc_context(reglet_t reglet);
void reglet_free_context(reg_ctx_t context);
void reglet_reset_context(reg_ctx_t context, char content[], size_t len);

// void reglet_

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  // __ACTRIE_REGEX_ENGINE_H__
