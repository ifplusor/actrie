/**
 * test_avl.c
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#include <alib/collections/map/avl.h>
#include <stdio.h>

struct node {
  avl_node_s avl_elem;
  int key;
};

sptr_t cmp(avl_node_t node, void* key) {
  struct node* n = container_of(node, struct node, avl_elem);
  return n->key - *(int*)key;
}

sptr_t walk_road(avl_node_t node, void* arg) {
  struct node* n = container_of(node, struct node, avl_elem);
  if (n->key < 20) {
    return -1;
  } else if (n->key > 30) {
    return 1;
  } else {
    return 0;
  }
}

void walk_op(avl_node_t node, void* arg) {
  struct node* n = container_of(node, struct node, avl_elem);
  printf("%d\n", n->key);
}

int main() {
  struct node node[32];
  avl_t avl = avl_construct(cmp);
  for (int i = 0; i < 32; i++) {
    node[i].key = i;
    avl_insert(avl, (void*)&i, &node[i].avl_elem);
  }
  avl_walk_in_order(avl, walk_road, walk_op, NULL, NULL);
  for (int i = 0; i < 32; i++) {
    avl_delete(avl, (void*)&i);
  }
  return 0;
}
