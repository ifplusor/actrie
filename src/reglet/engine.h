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
  trie_t trie;
} reglet_s, *reglet_t;

reglet_t reglet_construct();
void reglet_destruct(reglet_t reglet);

void reglet_add_pattern(reglet_t self, ptrn_t pattern, dstr_t extra);

reg_ctx_t reglet_alloc_context(reglet_t reglet);
void reglet_free_context(reg_ctx_t context);
void reglet_reset_context(reg_ctx_t context, char content[], size_t len);
void reglet_fix_pos(reg_ctx_t context, fix_pos_f fix_pos_func, void* fix_pos_arg);

void reglet_activate_expr_ctx(reg_ctx_t context);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  // __ACTRIE_REGEX_ENGINE_H__
