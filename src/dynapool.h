#ifndef _MATCH_NODE_POOL_H_
#define _MATCH_NODE_POOL_H_

#include "dlnk.h"

#if !DYNAPOOL_REGION_OFFSET
#define DYNAPOOL_REGION_OFFSET 8
#endif

#define DYNAPOOL_POSITION_MASK ((0x0001LL << DYNAPOOL_REGION_OFFSET) - 1)

typedef struct dynapool_node {
  deque_node_s header;
} dynapool_node_s, *dynapool_node_t;

typedef struct dyna_nodepool {
  void* _nodepool[REGION_SIZE];
  dynapool_node_s _sentinel;
  size_t node_size;
} dynapool_s, *dynapool_t;

dynapool_t dynapool_construct(size_t node_node);
#define dynapool_construct_with_type(type) dynapool_construct(sizeof(type))

bool dynapool_destruct(dynapool_t pool);

dynapool_node_t dynapool_alloc_node(dynapool_t pool);
bool dynapool_free_node(dynapool_t pool, dynapool_node_t node);

#endif //_MATCH_NODE_POOL_H_