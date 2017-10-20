//
// Created by james on 10/20/17.
//
#include "avl.h"

struct node {
  avl_node_s avl_elem;
  int key;
};

int cmp(avl_node_t node, void *key) {
  struct node *n = container_of(node, struct node, avl_elem);
  return n->key - (int)key;
}

void main() {
  struct node node[32];
  avl_t avl = avl_construct(cmp);
  for (int i = 0; i < 32; i++) {
    node[i].key = i;
    avl_insert(avl, (void*)i, &node[i].avl_elem);
  }
  for (int i = 0; i < 32; i++) {
    avl_delete(avl, (void*)(i));
  }
}
