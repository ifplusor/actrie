#include "dynapool.h"

const size_t DYNAPOOL_REGION_SIZE = REGION_SIZE;
const size_t DYNAPOOL_POSITION_SIZE = DYNAPOOL_POSITION_MASK + 1;

dynapool_t dynapool_construct(size_t node_node) {
  if (node_node == 0) return NULL;

  dynapool_t pool = malloc(sizeof(dynapool_s));
  if (pool == NULL) return NULL;

  for (size_t i = 0; i < DYNAPOOL_REGION_SIZE; i++)
    pool->_nodepool[i] = NULL;
  deque_init(&pool->_sentinel.header);
  pool->node_size = node_node;
}

bool dynapool_destruct(dynapool_t pool) {
  if (pool != NULL) {
    for (size_t i = 0; i < DYNAPOOL_REGION_SIZE; i++)
      free(pool->_nodepool[i]);
    free(pool);
  }
  return true;
}

dynapool_node_t dynapool_alloc_node(dynapool_t pool) {
  deque_node_t deque = &pool->_sentinel.header;
  if (deque_empty(deque)) {
    // extend memory
    for (size_t i = 0; i < DYNAPOOL_REGION_SIZE; i++) {
      if (pool->_nodepool[i] != NULL) continue;
      dynapool_node_t region = malloc(pool->node_size * DYNAPOOL_POSITION_SIZE);
      if (region == NULL) return NULL;
      size_t j = DYNAPOOL_POSITION_SIZE;
      while (j--) {
        dynapool_node_t node = region + pool->node_size * j;
        deque_push_front(deque, node, dynapool_node_s, header);
      }
      break;
    }
  }
  return deque_pop_front(deque, dynapool_node_s, header);
}

bool dynapool_free_node(dynapool_t pool, dynapool_node_t node) {
  if (pool == NULL || node == NULL) return false;
  deque_push_front(&pool->_sentinel.header, node, dynapool_node_s, header);
  return true;
}
