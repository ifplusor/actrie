//
// Created by james on 10/17/17.
//

#ifndef _ACTRIE_MDIMAP_H_
#define _ACTRIE_MDIMAP_H_

#include "avl.h"
#include "dlnk.h"

#include "dict0.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct mdi_map_node;
typedef struct mdi_map_node *mdim_node_t;

typedef struct mdi_map_node {
  mdi_t idx;
  strpos_s pos;

  mdim_node_t next;
  mdim_node_t last;

  avl_node_s avl_elem;
  deque_node_s deque_elem; // queue for context out
} mdim_node_s;

typedef avl_t mdimap_t;

mdimap_t mdimap_construct(bool replace);
bool mdimap_destruct(mdimap_t map);
bool mdimap_reset(mdimap_t map);
mdim_node_t mdimap_search(mdimap_t map, void *key);
bool mdimap_insert(mdimap_t map, mdim_node_t node);
bool mdimap_delete(mdimap_t map, mdim_node_t node);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //_ACTRIE_MDIMAP_H_
