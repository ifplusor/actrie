//
// Created by james on 9/28/17.
//

#ifndef _MATCH_AVL_H_
#define _MATCH_AVL_H_

#include <common.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if !AVL_EXTEND_SPACE_SIZE
#define AVL_EXTEND_SPACE_SIZE 16
#endif

#if !AVL_USER_INDEX
#define AVL_USER_INDEX 1
#define size_t int
#endif

#if AVL_USER_INDEX
typedef size_t avl_node_t;
#else
struct avl_node;
typedef struct avl_node *avl_node_t;
#endif

typedef struct avl_node {
  void *data;
  avl_node_t left, right, parent;
  int bf; // balance factor
} avl_node_s;

typedef int (*avl_cmp_data)(void *a, void *b);
typedef void (*avl_set_data)(void **container, void *data);

typedef struct avl {
  size_t _size, _len;
  avl_node_s *_nodepool;
#define avl_sentinel _nodepool[0]
#define avl_root avl_sentinel.right
#if AVL_USER_INDEX
#define avl_invalid_node(self) 0
#else
#define avl_invalid_node(self) self->_nodepool
#endif

#define avl_free_leader(self) self->_nodepool[1]
#define avl_free_next(self) self->_nodepool[1].right
#if AVL_USER_INDEX
#define avl_free_leader_addr(self) 1
#else
#define avl_free_leader_addr(self) &avl_free_leader(self)
#endif
#define avl_free_empty(self) avl_free_leader_addr(self)

  avl_cmp_data cmp_data;
  avl_set_data set_data;
} avl_s, *avl_t;

bool avl_init(avl_t self, size_t size);
void avl_remove_all(avl_t self);
bool avl_insert(avl_t self, void *data);
void *avl_search(avl_t self, void *data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //_MATCH_AVL_H_
