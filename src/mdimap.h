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

  mdi_s buf[0]; // dynamic size for idx buffer
} mdim_node_s;

typedef enum mdim_bucket_type {
  mdim_bucket_type_empty = 0,
  mdim_bucket_type_direct = 1,
  mdim_bucket_type_avl = 2
} mdim_bucket_type_e;

typedef struct mdi_map_bucket {
  union {
    avl_t avl;
    mdim_node_t data;
  } store;
  mdim_bucket_type_e type;
} mdim_bucket_s, *mdim_bucket_t;

typedef struct mdi_map {
  mdim_bucket_s *bucket;
  size_t size, len, rank;
  bool replace;
  bool rehash;
} mdimap_s, *mdimap_t;

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
