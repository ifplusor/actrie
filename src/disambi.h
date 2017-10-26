//
// Created by james on 9/27/17.
//

#ifndef _ACTRIE_DISAMBI_H_
#define _ACTRIE_DISAMBI_H_

#include "acdat.h"
#include "dynapool.h"
#include "dlnk.h"
#include "mdimap.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct ambi_matcher {
  struct _matcher header;

  matcher_t _pure_matcher;
  match_dict_t _dict;
} *ambi_matcher_t;

typedef struct ambi_context {
  struct _context header; /* 'header.out_matched_index' point 'out_index' */

  ambi_matcher_t _matcher;

  context_t _pure_ctx;

  dynapool_t _mdiqn_pool;

  mdimap_t _word_map;
  mdimap_t _ambi_map;

  deque_node_s _out_buffer;
  mdi_s out_index;

} ambi_context_s, *ambi_context_t;

typedef struct ambi_config {
  aobj pure;
} ambi_config_s, *ambi_config_t;

aobj ambi_matcher_conf(uint8_t id, aobj pure);

matcher_t ambi_construct(match_dict_t dict, aobj conf);
bool ambi_destruct(ambi_matcher_t self);

ambi_context_t ambi_alloc_context(ambi_matcher_t matcher);
bool ambi_free_context(ambi_context_t context);
bool ambi_reset_context(ambi_context_t context, unsigned char content[], size_t len);

bool ambi_next_on_index(ambi_context_t ctx);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //_ACTRIE_DISAMBI_H_
